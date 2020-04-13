/*
 *  thx for flags lib https://github.com/sailormoon/flags
*/
#include <lib/flags.hpp>

#include <iostream>
#include <fstream>
#include <cstring>
#include <random>
#include <thread>

#include "picojson.h"
#include "UdtListener.hpp"
#include "UdpSocket.hpp"

constexpr uint32_t maximum_file_buffer = 1024;
constexpr uint32_t magick_word = 999888;
constexpr uint16_t default_recv_port = 21789;
constexpr uint16_t default_send_port = 21790;

constexpr const char *natserver_ip = "217.182.21.102";
constexpr uint16_t natserver_port = 2000;

enum class MsgTypes {
    FIND = 1000,
    CONNECTED = 1001,
    CANNOTCONNECT = 1002,
    P2PACCEPT = 1003,
    GOODBYE = 1004,
    MMSTREAM = 1005
};

static uint32_t client_id;

struct msg_initial_t {
    uint32_t m_magick_word;
    uint64_t m_filesize;
    char m_filename[128];
};

void showHelp()
{
    std::cout << "Help on https://github.com/regular-dev/puppytransfer readme file\n";
}

std::pair< std::string, uint16_t > punchNat(const std::string &nat_word)
{
    const std::string nat_prefix("puppytransfer1.0_");
    std::pair< std::string, uint16_t > wrong("", 0);

    UdpSocket s_transport;
    UdpSocket s_msg;
    s_transport.bind("0.0.0.0", default_recv_port);
    s_msg.bind("0.0.0.0", default_send_port);
    s_msg.setNonBlocking(true);

    // from transport socket
    picojson::object json_init;
    json_init["client_id"] = picojson::value(static_cast< double >(client_id));
    json_init["type"] = picojson::value(static_cast< double >(MsgTypes::MMSTREAM));
    json_init["matchpass"] = picojson::value(nat_prefix + nat_word);

    std::string str_json_init = picojson::value(json_init).serialize();

    s_msg.sendTo(str_json_init.c_str(), str_json_init.size(), natserver_ip, natserver_port);

    // from msg socket
    json_init["type"] = picojson::value(static_cast< double >(MsgTypes::FIND));
    str_json_init = picojson::value(json_init).serialize();

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(250ms);

    s_transport.sendTo(str_json_init.c_str(), str_json_init.size(), natserver_ip, natserver_port);

    std::this_thread::sleep_for(1s);

    // receive msg socket
    char buf_msg[512];
    std::string remote_server_ip;
    uint16_t remote_server_port;

    std::string str_buf;
    picojson::value json_recvd;

    std::cout << "Waiting for response from NAT hole punching server...\n";
    while (1) {
        if (s_msg.recvFrom(buf_msg, 512, remote_server_ip, remote_server_port) < 0) {
            std::this_thread::sleep_for(2s);
            
            json_init["type"] = picojson::value(static_cast< double >(MsgTypes::FIND));
            str_json_init = picojson::value(json_init).serialize();
            // for MsgTypes::FIND
            s_transport.sendTo(str_json_init.c_str(), str_json_init.size(), natserver_ip, natserver_port);

            // for MsgTypes::MMSTREAM
            json_init["type"] = picojson::value(static_cast< double >(MsgTypes::MMSTREAM));
            str_json_init = picojson::value(json_init).serialize();
			std::this_thread::sleep_for(250ms);

            s_msg.sendTo(str_json_init.c_str(), str_json_init.size(), natserver_ip, natserver_port);
        }
        else {
            str_buf = buf_msg;
            std::string parse_errs = picojson::parse(json_recvd, str_buf);

             if (!parse_errs.empty()) {
                std::cerr << "ERROR : Failed parsing received json from nat hole server...";
                return wrong;
            }

            if (json_recvd.get<picojson::object>()["type"].get< double >() == 1007) {
                std::cout << "Received remote peer address...\n";
                break;
            }
		}
    }
   
    std::string remote_client_ip;
    uint16_t remote_client_port = 0;
    bool difference = false;

    const picojson::value::object &obj = json_recvd.get< picojson::object >();
    for (picojson::value::object::const_iterator i = obj.begin();
         i != obj.end();
         ++i) {
        if (i->first == "ip") {
            remote_client_ip = i->second.to_str();
        }
        if (i->first == "port") {
            remote_client_port = static_cast< uint16_t >(atoi(i->second.to_str().c_str()));
        }
        if (i->first == "cs") {
            if (i->second.to_str() == "server")
                difference = false;
            else
                difference = true;
        }
    }

    if (remote_client_port == 0 || remote_client_ip.empty()) {
        std::cerr << "ERROR : got invalid remote ip or port from nat server \n";
        return wrong;
    }

    if (difference)
        std::this_thread::sleep_for(3s);

    // send to remote side
    json_init["type"] = picojson::value(static_cast< double >(MsgTypes::P2PACCEPT));
    str_json_init = picojson::value(json_init).serialize();

    // reliability :)
    s_transport.sendTo(str_json_init.c_str(), str_json_init.size(), remote_client_ip, remote_client_port);
    s_transport.sendTo(str_json_init.c_str(), str_json_init.size(), remote_client_ip, remote_client_port);
    std::this_thread::sleep_for(25ms);
    s_transport.sendTo(str_json_init.c_str(), str_json_init.size(), remote_client_ip, remote_client_port);
    s_transport.sendTo(str_json_init.c_str(), str_json_init.size(), remote_client_ip, remote_client_port);
    std::this_thread::sleep_for(500ms);

    if (s_transport.recvFrom(buf_msg, 512, remote_client_ip, remote_client_port) < 0)
        return wrong;

    std::cout << "Succesfully punced NAT ! \n";

    return std::pair< std::string, uint16_t >(remote_client_ip, remote_client_port);
}

void sendFileBasic(const std::vector< std::string_view > &files,
                   const std::string &remote_ip, uint16_t port = default_send_port)
{
    UdtSocket udt_sock;

    if (udt_sock.listen(default_recv_port) < 0) {
        std::cerr << "ERROR : Couldn't listen with UdtSocket on port!! \n";
    }

    if (udt_sock.connect(remote_ip, port) < 0) {
        std::cerr << "ERROR : Couldn't connect to remote node \n";
        return;
    }

    uint32_t files_cound = static_cast< uint32_t >(files.size());
    udt_sock.send(reinterpret_cast< char * >(&files_cound), sizeof(uint32_t));

    for (const auto &file : files) {

        msg_initial_t initial;

        initial.m_magick_word = magick_word;
        std::memcpy(initial.m_filename, file.data(), file.size());
        initial.m_filename[file.size()] = '\0';

        std::ifstream in_file;
        in_file.open(file.data(), std::ios::binary | std::ios::ate);
        initial.m_filesize = static_cast< size_t >(in_file.tellg());

        udt_sock.send(reinterpret_cast< char * >(&initial), sizeof(initial));

        // sending file
        uint64_t current_buf_size = maximum_file_buffer;
        uint64_t sended_bytes = 0;
        char file_buf[maximum_file_buffer];
        in_file.seekg(0, std::ios::beg);

        while (sended_bytes < initial.m_filesize) {
            if ((initial.m_filesize - sended_bytes) < maximum_file_buffer)
                current_buf_size = initial.m_filesize - sended_bytes;

            in_file.read(file_buf, static_cast< long >(current_buf_size));
            udt_sock.send(file_buf, current_buf_size);

            sended_bytes += maximum_file_buffer;
        }

        in_file.close();
        std::cout << "Succesfully sent file : " << file << "\n";
    }
}

void receiveFileBasic(uint16_t listen_port = default_recv_port)
{
    UdtListener listener;
    listener.listen(listen_port);
    auto s = listener.accept();

    // receive number of files
    uint32_t files_count;
    if (s.recv(reinterpret_cast< char * >(&files_count), sizeof(files_count)) < 0) {
        std::cerr << "ERROR : Couldn't receive files count... \n";
        return;
    }

    for (uint32_t i = 0; i < files_count; ++i) {
        msg_initial_t initial;

        s.recv(reinterpret_cast< char * >(&initial), sizeof(initial));

        if (initial.m_magick_word != magick_word) {
            std::cerr << "ERROR : Received invalid magick word, exiting... \n";
            return;
        }

        std::string str_file_name(initial.m_filename, strlen(initial.m_filename));
        std::ofstream out_file(str_file_name);

        std::cout << "Receiving file... \n";
        std::cout << "File name : " << str_file_name << "\n";
        std::cout << "File size : " << initial.m_filesize << "\n";

        char file_buf[maximum_file_buffer];
        uint64_t current_buf_size = maximum_file_buffer;
        uint64_t recved_bytes = 0;

        while (recved_bytes < initial.m_filesize) {
            if ((initial.m_filesize - recved_bytes) < maximum_file_buffer)
                current_buf_size = initial.m_filesize - recved_bytes;

            s.recv(file_buf, current_buf_size);
            out_file.write(file_buf, static_cast< long >(current_buf_size));
            recved_bytes += current_buf_size;
        }

        out_file.close();
        std::cout << "Successfully received file (" + str_file_name + ") \n";
    }
}

int main(int argc, char *argv[])
{
    const flags::args args(argc, argv);

    const auto flag_send = args.get_option_exists("send");
    const auto flag_recv = args.get_option_exists("receive");
    const auto flag_nat_word = args.get< std::string >("nat_word");
    const auto flag_ip = args.get< std::string >("ip");
    const auto flag_port = args.get< uint16_t >("port");
    const auto arg_files = args.positional();

    std::random_device rand_rd;
    std::mt19937 rand_mt(rand_rd());
    std::uniform_int_distribution<uint32_t> rand_dist(1, 1000000);
    client_id = rand_dist(rand_mt);

    if ((!flag_send && !flag_recv) || (flag_send && flag_recv)) {
        std::cout << "Please enter either --send or --receive flag \n";
        showHelp();
        return -1;
    }

    // send file //
    if (flag_send) {
        if (arg_files.size() < 1) {
            std::cout << "Please enter at least one file to send !";
            showHelp();
            return -1;
        }

        if (flag_nat_word) {
            const auto remote_addr = punchNat(flag_nat_word.value());

            if (remote_addr.second == 0) {
                std::cerr << "ERROR : Failed NAT hole punching... \n";
                return -1;
            }

            std::cout << remote_addr.first << "  :  " << remote_addr.second << "\n";

            sendFileBasic(arg_files, remote_addr.first, remote_addr.second);
            return 0;
        }

        if (flag_ip) {
            flag_port ? sendFileBasic(arg_files, flag_ip.value(), flag_port.value()) :
            sendFileBasic(arg_files, flag_ip.value());
            return 0;
        } else {
            std::cerr << "Please, enter --ip and --port \n";
            return -1;
        }
    }

    // receive file //
    if (flag_recv) {
        if (flag_nat_word) {
            const auto remote_addr = punchNat(flag_nat_word.value());

            if (remote_addr.second == 0) {
                std::cerr << "ERROR : Failed NAT hole punching... \n";
                return -1;
            }

            receiveFileBasic();
            return 0;
        }

        flag_port ? receiveFileBasic(flag_port.value()) :
        receiveFileBasic();
    }

    return 0;
}
