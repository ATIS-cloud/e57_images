# E57 File Generator with Embedded Images

This folder contains a Python script to generate E57 files with embedded point cloud data and images. It ensures compliance with industry standards and facilitates seamless integration with platforms such as Autodesk ReCap 2025 and ATIS.cloud.

Python code that writes E57 files : [unstructured_e57_with_images.py](unstructured_e57_with_images.py)

## Features
- Generates E57 files with embedded images.
- Complies with modern standards for unstructured E57 files.
- Easy setup and compatibility with both Windows and Linux systems.

## Prerequisites
Ensure you have the following before running the script:
1. Python 3.10 or newer installed on your system.
2. `pip` package manager to install dependencies.

Using a virtual environment is optional but recommended to avoid dependency conflicts.

## Installation and Execution

### Windows
1. **Navigate to the Script Directory**:
   ```
   cd example/code/python
   ```

2. **(Optional) Set Up a Virtual Environment**:
   ```
   pip install virtualenv
   virtualenv venv
   venv\Scripts\activate
   ```

3. **Install Dependencies**:
   ```
   pip install -r requirements.txt
   ```

4. **Run the Script**:
   ```
   python unstructured_e57_with_images.py
   ```

### Linux
1. **Navigate to the Script Directory**:
   ```bash
   cd example/code/python
   ```

2. **(Optional) Set Up a Virtual Environment**:
   ```bash
   pip3 install virtualenv
   virtualenv venv
   source venv/bin/activate
   ```

3. **Install Dependencies**:
   ```bash
   pip3 install -r requirements.txt
   ```

4. **Run the Script**:
   ```bash
   python3 unstructured_e57_with_images.py
   ```

## Troubleshooting
- **Missing Virtual Environment Command**: Ensure `virtualenv` is installed using:
  ```bash
  pip install virtualenv
  ```

- **Dependency Issues**: Verify the `requirements.txt` file contains all necessary dependencies.

## Contributing
Contributions are welcome! Follow these steps to contribute:

1. Fork the repository.
2. Create a feature branch.
3. Submit a pull request with your changes.

## License
This project is licensed under the MIT License. See the LICENSE file for details.
