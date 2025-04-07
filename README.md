# Simple TCP-Based Web Server

## Overview

This project is a basic TCP web server implemented in C. It was developed as part of a university assignment and is intended for educational purposes only. The server listens on a predefined port (`8080`), accepts multiple client connections, and serves files from the current working directory. When accessed via a browser, it generates a simple HTML index listing all available files, and allows downloading supported file types.

> ⚠️ **Note:** This project is no longer actively maintained. It serves as a learning example and may not follow best practices for robust or secure web server implementation.

---

## Features

- Handles multiple client connections using TCP sockets
- Dynamically generates an HTML index of the available files
- Serves local files via HTTP responses
- Supports basic MIME type handling based on file extensions
- Basic error handling (e.g., returns 404 for unsupported or missing files)

---

## File Structure

```
.
├── Makefile        # Build instructions
├── README.md       # Project description (this file)
├── servTcpIt.c     # Main server implementation
```

---

## Build Instructions

You can build the server using `make`:

```bash
make
```

To clean up the generated binary:

```bash
make clean
```

---

## Usage

After building the project, run the server:

```bash
./serverTCP
```

The server will:
- Listen on port `8080`
- Serve files in the current working directory
- Display a list of all files (except itself and special files) at the root URL

You can then access it from a web browser:

```
http://localhost:8080
```

---

## Limitations & Known Issues

- File type detection is simplistic and based solely on file extensions
- Large files are not streamed; they are loaded fully into memory
- Only a few MIME types are recognized
- No HTTPS support
- Minimal logging and error reporting
- Basic memory management (some leaks may be present)

---

## Disclaimer

This server is intended for demonstration and educational purposes only. It lacks many security features and should not be used in production environments.

---

## License

This project does not include a formal license. Please contact the author for permission before redistributing or using this work beyond academic or personal experimentation.
