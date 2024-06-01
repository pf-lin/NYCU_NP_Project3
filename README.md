# Network Programming Project 3 - HTTP Server and CGI Programs

The project is divided into **two parts**.  

1. Write a simple HTTP server called **http_server** and a CGI program **console.cgi**. It should run on **NP Linux Workstation**.
2. Provides the same functionality as part 1, but with some rules slightly differs:
   - Implement one program, **cgi_server.exe**, which is a combination of **http_server**, **panel.cgi**, and **console.cgi**.
   - Should run on **Windows 10**.

*Use **Boost.Asio** library to accomplish this project.*

## Compile

### Part 1

#### Build

```
make part1
```

#### Execution

1. Run **np_single_golden** on the corresponding NP server
2. Run **http_server** by

   ```
   ./http_server [port]
   ```
   
3. Open a browser and visit `http://[HTTP_server_host]:[port]/panel.cgi`
4. Fill the form with the servers to connect to and select the input file, then click **Run**.
5. The web page will automatically redirected to `http://[NP_server_host]:[port]/console.cgi` and your **console.cgi** should start now.

### Part 2

#### Build

```
make part2
```

#### Execution

1. Run **np_single_golden** on the corresponding NP server
2. Run **cgi_server.exe** by

   ```
   .\cgi_server.exe [port]
   ```
   
3. Open a browser and visit `http://[HTTP_server_host]:[port]/panel.cgi`
4. Fill the form with the servers to connect to and select the input file, then click **Run**.
5. The web page will automatically redirected to `http://[NP_server_host]:[port]/console.cgi` and your **console.cgi** should start now.
