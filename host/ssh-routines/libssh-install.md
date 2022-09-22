
# Build requirements

sudo apt install ...

- A C compiler / build-essential
- [CMake](https://www.cmake.org) >= 2.6.0 / cmake
- [openssl](https://www.openssl.org) >= 0.9.8 / openssh-server
- [gcrypt](https://www.gnu.org/directory/Security/libgcrypt.html) >= 1.4 / libgcrypt-dev
- [libz](https://www.zlib.net) >= 1.2 / zlib1g-dev
- [cmocka](https://cmocka.org/) >= 1.1.0 / libcmocka-dev

# CMake and make
mkdir build && cd build
cmake -DUNIT_TESTING=ON -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Debug ..
make -j8

# cd to examples and run ssh
