# E57 File Generator with Embedded Images

This folder contains a C++ solution to generate E57 files with embedded point cloud data and images. It ensures compliance with industry standards and facilitates seamless integration with platforms such as Autodesk ReCap 2025 and ATIS.cloud.

C++ code that writes E57 files: [DemoWrite01.cpp](src/examples/DemoWrite01.cpp)

## Features
- Generates E57 files with embedded images.
- Complies with modern standards for unstructured E57 files.
- Easy setup using Docker for dependency management.

## Requirements
Ensure you have the following dependencies if not using Docker:

- Xerces 3.1.0
- Boost 1.83.0

## Build and Execution

### Build and Run Using Docker
This repository includes a Dockerfile to simplify the build process and handle dependencies.

Navigate to the C++ code directory:
````bash
cd code/C++
````
Build the Docker image:
````bash
docker build -t build_libe57 .
````
Run the Docker container:
````bash
docker run -v <repo_root_dir>/code/C++:/app -t build_libe57
````

### Build the C++ Solution Inside the Container
Once inside the Docker container, execute the following commands:
````bash
mkdir build && cd build && cmake ../ && make
````

## Troubleshooting
**Dependency Issues Outside Docker**:
Ensure that the required versions of Xerces and Boost are installed and properly linked in your build environment.

## Contributing
Contributions are welcome! Follow these steps to contribute:

1. Fork the repository.
2. Create a feature branch.
3. Submit a pull request with your changes.

## License
This project is licensed under the MIT License. See the LICENSE file for details.
