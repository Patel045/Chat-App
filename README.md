# Chat-App

# Project Structure
```
 - chatClient/
    - async_client.cpp         # Client implementation
    - echo_client.cpp          # Use for Testing
 - chatServer/
    - async_server.cpp         # Server implementation
    - echo_server.cpp          # Use for Testing
 - networkLibrary/
    - networkLibrary.h         # Network library header
    - networkLibrary.cpp       # Network library implementation
 - utils/
    - Logger.h                 # Logger utility header
    - Logger.cpp               # Logger utility implementation
 - CMakeLists.txt             # Root CMake configuration
```

# Components

 - chatClient: The client module that communicates with the server.
 - chatServer: The server module that handles multiple clients asynchronously.
 - networkLibrary: A library encapsulating network communication logic.
 - utils: A utility module that provides logging capabilities across the application.

# Requirements

 - C++17 or later
 - CMake 3.10 or higher
 - Git (for cloning the repository)
 - A C++ compiler that supports C++17 (e.g., g++, clang++)

# Setup Instructions
## 1. Clone the Repository

To clone this repository, use the following command:

```
git clone https://github.com/yourusername/ChatApplication.git
cd ChatApplication
```

## 2. Build the Project

To build the project, follow these steps:

```
# Create a build directory
mkdir build
cd build

# Run CMake to configure the project
cmake ..

# Build the project using CMake
cmake --build .
```

## 3. Run the Executables

After building the project, you can run the chat server and client as follows:

### Server : 
```
./chatServer/chatServer <SERVER PORT>
```

### Client :
```
./chatClient/chatClient <SERVER IP> <SERVER PORT>
```

# 5. Clean Up

To remove all build artifacts, you can clean the build directory:

```
cd build
cmake --build . --target clean
```

# Contributing

Contributions are welcome! Please fork the repository and create a pull request with your changes. Ensure your code follows the existing style and passes all tests before submitting.

# License

This project is licensed under the MIT License. See the LICENSE file for details.


# Contact

If you have any questions or need further assistance, please feel free to reach out:
 - Email: aayushpatel2904@gmail.com
 - GitHub: Patel045