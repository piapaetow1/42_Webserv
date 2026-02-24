# Webserv

A custom HTTP/1.1 web server written in C++ as part of the 42 curriculum.

This project recreates core functionalities of a real web server such as Nginx,
including request parsing, configuration handling, multiple ports, and CGI execution.

---

## 🚀 Features

- HTTP/1.1 compliant
- Supports:
  - GET
  - POST
  - DELETE
- Multiple server blocks with different ports
- Config file similar to Nginx
- File serving (static content)
- Error pages
- CGI support
- Fully custom request parsing
- Non-blocking I/O with poll()
- Dynamic frontend: Shopping List application
- External API integration (TheMealDB)

---

## 🛒 Special Frontend: Dynamic Shopping List

Unlike a static demo page, our frontend is a fully interactive shopping list:

- Add items via POST
- Delete items via DELETE
- Each configured port runs its own independent shopping list
- Data is handled server-side

### 🍽 Get Inspiration Feature

The frontend integrates with the public API:

TheMealDB

Users can:
- Fetch meal ideas
- Get inspiration for cooking
- Dynamically add ingredients to their shopping list

This demonstrates:
- External API calls
- Dynamic content handling
- Real-world HTTP interaction

---

## 🏗 Compilation

On Linux: 

```bash
make
```

This will generate the executable:
./webserv

Start the server with a configuration file:
./webserv configFiles/valid.conf

Send requests via Terminal or go to http://localhost:8080


This is a group project created in cooperation with Reu-si and phteeven1.