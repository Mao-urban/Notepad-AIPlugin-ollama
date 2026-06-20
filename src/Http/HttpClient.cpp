//this file is part of notepad_aiplug
//Copyright (C)2025

#include "HttpClient.h"
#include <windows.h>
#include <winhttp.h>
#include <sstream>
#include <stdexcept>

#pragma comment(lib, "winhttp.lib")

HttpClient::HttpClient()
    : _timeoutMs(600000) // 30 minutes default timeout
{
}

HttpClient::~HttpClient()
{
}

void HttpClient::setHeader(const std::string& name, const std::string& value)
{
    _headers[name] = value;
}

void HttpClient::clearHeaders()
{
    _headers.clear();
}

void HttpClient::setTimeout(int timeoutMs)
{
    _timeoutMs = timeoutMs;
}

std::string HttpClient::post(const std::string& url, const std::string& body)
{
    return performRequest(url, "POST", body);
}

std::string HttpClient::get(const std::string& url)
{
    return performRequest(url, "GET", "");
}

std::wstring HttpClient::toWideString(const std::string& str)
{
    if (str.empty())
        return std::wstring();

    int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    std::wstring wstr(len - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], len);
    return wstr;
}

bool HttpClient::parseUrl(const std::string& url, std::wstring& host, std::wstring& path, int& port, bool& isHttps)
{
    std::string urlLower = url;
    for (auto& c : urlLower)
        c = tolower(c);

    isHttps = (urlLower.find("https://") == 0);
    bool isHttp = (urlLower.find("http://") == 0);

    if (!isHttps && !isHttp)
    {
        _lastError = "Invalid URL scheme";
        return false;
    }

    size_t schemeEnd = isHttps ? 8 : 7;
    std::string remainder = url.substr(schemeEnd);

    // Find path start
    size_t pathStart = remainder.find('/');
    std::string hostPort;
    std::string pathStr;

    if (pathStart != std::string::npos)
    {
        hostPort = remainder.substr(0, pathStart);
        pathStr = remainder.substr(pathStart);
    }
    else
    {
        hostPort = remainder;
        pathStr = "/";
    }

    // Check for port
    size_t colonPos = hostPort.find(':');
    if (colonPos != std::string::npos)
    {
        host = toWideString(hostPort.substr(0, colonPos));
        port = std::stoi(hostPort.substr(colonPos + 1));
    }
    else
    {
        host = toWideString(hostPort);
        port = isHttps ? 443 : 80;
    }

    path = toWideString(pathStr);
    return true;
}

std::string HttpClient::performRequest(const std::string& url, const std::string& method, const std::string& body)
{
    std::wstring host, path;
    int port;
    bool isHttps;

    if (!parseUrl(url, host, path, port, isHttps))
    {
        throw std::runtime_error(_lastError);
    }

    // Initialize WinHTTP
    HINTERNET hSession = WinHttpOpen(
        L"NotepadPlusPlus-AI-Plugin/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        0);

    if (!hSession)
    {
        _lastError = "Failed to initialize WinHTTP";
        throw std::runtime_error(_lastError);
    }

    // Set timeouts
    WinHttpSetTimeouts(hSession, _timeoutMs, _timeoutMs, _timeoutMs, _timeoutMs);

    // Connect
    HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(), port, 0);
    if (!hConnect)
    {
        WinHttpCloseHandle(hSession);
        _lastError = "Failed to connect to server";
        throw std::runtime_error(_lastError);
    }

    // Open request
    DWORD flags = isHttps ? WINHTTP_FLAG_SECURE : 0;
    std::wstring methodW = toWideString(method);

    HINTERNET hRequest = WinHttpOpenRequest(
        hConnect,
        methodW.c_str(),
        path.c_str(),
        NULL,
        WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES,
        flags);

    if (!hRequest)
    {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        _lastError = "Failed to open request";
        throw std::runtime_error(_lastError);
    }

    // Add headers
    for (const auto& header : _headers)
    {
        std::wstring headerLine = toWideString(header.first + ": " + header.second);
        WinHttpAddRequestHeaders(hRequest, headerLine.c_str(), -1, WINHTTP_ADDREQ_FLAG_ADD);
    }

    // Send request
    BOOL result;
    if (method == "POST" && !body.empty())
    {
        result = WinHttpSendRequest(
            hRequest,
            WINHTTP_NO_ADDITIONAL_HEADERS,
            0,
            (LPVOID)body.c_str(),
            (DWORD)body.length(),
            (DWORD)body.length(),
            0);
    }
    else
    {
        result = WinHttpSendRequest(
            hRequest,
            WINHTTP_NO_ADDITIONAL_HEADERS,
            0,
            WINHTTP_NO_REQUEST_DATA,
            0,
            0,
            0);
    }

    if (!result)
    {
        DWORD error = GetLastError();
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        _lastError = "Failed to send request, error code: " + std::to_string(error);
        throw std::runtime_error(_lastError);
    }

    // Receive response
    result = WinHttpReceiveResponse(hRequest, NULL);
    if (!result)
    {
        DWORD error = GetLastError();
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        _lastError = "Failed to receive response, error code: " + std::to_string(error);
        throw std::runtime_error(_lastError);
    }

    // Check status code
    DWORD statusCode = 0;
    DWORD statusCodeSize = sizeof(statusCode);
    WinHttpQueryHeaders(
        hRequest,
        WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
        WINHTTP_HEADER_NAME_BY_INDEX,
        &statusCode,
        &statusCodeSize,
        WINHTTP_NO_HEADER_INDEX);

    // Read response data
    std::string response;
    DWORD bytesAvailable = 0;
    DWORD bytesRead = 0;

    do
    {
        bytesAvailable = 0;
        WinHttpQueryDataAvailable(hRequest, &bytesAvailable);

        if (bytesAvailable > 0)
        {
            std::vector<char> buffer(bytesAvailable + 1, 0);
            WinHttpReadData(hRequest, buffer.data(), bytesAvailable, &bytesRead);
            response.append(buffer.data(), bytesRead);
        }
    } while (bytesAvailable > 0);

    // Cleanup
    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    // Check for error status codes
    if (statusCode >= 400)
    {
        _lastError = "HTTP error " + std::to_string(statusCode) + ": " + response;
        throw std::runtime_error(_lastError);
    }

    return response;
}
