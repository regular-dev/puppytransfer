# PuppyTransfer

PuppyTransfer is linux CLI utility, which allows to send files P2P in LAN, and also through most NATs.

  - Based on UDT
  - No dependencies
  - Requires C++11

### Installation

You can download AppImage, and also it will be soon available on flatpack and snap.

### Building

Build with cmake :

```sh
cmake <path_to_src>
make
```

### Using examples

In local network :

```sh
./puppytransfer --receive #on receiving file host
./puppytransfer --send --ip=127.0.0.1 image1.png image2.jpg document.zip # on sending host
```
In public internet, we need to punch NAT to establish P2P connection, so you need to choose some keyword, that will identify your connection :

```sh
./puppytransfer --receive --nat_word=puppy_
./puppytransfer --send --nat_word=puppy_ # the same nat keyword
```

See [regular_website](http://regular.viewdns.net)
