# Arduino Library for network connections management hello

Library for handling and managing network connections by providing keepAlive functionalities and reconnection attempts. Initially for WiFi, GSM and NB, its goal is to be extended to more networking layers over diverse hardware (Ethernet, BlueTooth et al.)


## Usage

## Credits

## Website

# License

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA


### ConnectionHandler

**Connection Handler** is configured via a series of compiler directives, including the correct implementation of the class based on which board is selected.

### How to use it
- Instantiate the class with `ConnectionHandler conHandler(SECRET_SSID, SECRET_PASS);` if you are using a WiFi board, otherwise replace **WiFi** with **GSM** or **NB** or any future implementation minding to carefully pass the init parameters to the constructor (i.e.: for the GSM you'll need to pass PIN, APN, login, password).

- The `update()` method does all the work. It uses a finite state machine and is responsible for connection and reconnection to a network. The method is designed to be non-blocking by using time (milliseconds) to perform its tasks.

- `&getClient()` returns a reference to an instance of the `Client` class used to connect to the network.

- `getStatus()` returns the network connection status. The different states are defined in an `enum`
