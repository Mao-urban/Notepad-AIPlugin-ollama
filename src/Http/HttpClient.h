//this file is part of notepad_aiplug
//Copyright (C)2025

#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <string>
#include <map>
#include <functional>

class HttpClient
{
public:
    HttpClient();
    ~HttpClient();

    // Set request headers
    void setHeader(const std::string& name, const std::string& value);

    // Clear all headers
    void clearHeaders();

    // Perform POST request
    std::string post(const std::string& url, const std::string& body);

    // Perform GET request
    std::string get(const std::string& url);

    // Set timeout in milliseconds
    void setTimeout(int timeoutMs);

    // Get last error message
    std::string getLastError() const { return _lastError; }

private:
    std::map<std::string, std::string> _headers;
    int _timeoutMs;
    std::string _lastError;

    // Parse URL into components
    bool parseUrl(const std::string& url, std::wstring& host, std::wstring& path, int& port, bool& isHttps);

    // Convert string to wstring
    std::wstring toWideString(const std::string& str);

    // Perform HTTP request
    std::string performRequest(const std::string& url, const std::string& method, const std::string& body);
};

#endif // HTTPCLIENT_H
