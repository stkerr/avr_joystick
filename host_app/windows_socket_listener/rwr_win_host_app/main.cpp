#define WIN32_LEAN_AND_MEAN
// I'm going to use old APIs and you'll like it
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <hidsdi.h>
#include <hidclass.h>
#include <setupapi.h>
#include <tchar.h>
#include <string>
#include <stdio.h>
#include <vector>

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "rapidjson/pointer.h"
#include "rapidjson/rapidjson.h"
#include "rapidjson/error/en.h"

#ifndef _WIN32
#define _WIN32
#endif

extern "C" {
    #include "report.h"
}

void print_error(int error)
{
    LPTSTR pTemp = NULL;

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ARGUMENT_ARRAY,
        NULL,
        error,
        LANG_NEUTRAL,
        (LPTSTR)&pTemp,
        0,
        NULL
        );


    _tprintf(_T("%s\n"), pTemp);
}

enum track_type
{
    UNKNOWN,
    SCAN,
    SCAN_AND_TRACK,
    MISSILE_ACTIVE_HOMING
};

typedef struct _tracking_target {
    enum TYPE type;
    enum track_type signal_type;
    double priority;
    double azimuth;
    double power;
} tracking_target;

#pragma pack(push,1)
struct report_wrapper {
    BYTE report_number;
    USB_JoystickReport_Data_t report;
};
#pragma pack(pop)

// 1 MB on the heap
char input[1024 * 1024];

static const char* kTypeNames[] =
{ "Null", "False", "True", "Object", "Array", "String", "Number" };

int main(int argc, char** argv)
{
    std::wstring rwr_device_name;

    // Get the GUID of HID devices
    GUID hid_guid;
    HidD_GetHidGuid(&hid_guid);

    HDEVINFO device_info_set = SetupDiGetClassDevs(
        &hid_guid,
        0,
        0,
        DIGCF_PRESENT | DIGCF_DEVICEINTERFACE
        );

    DWORD index = 0;
    BOOL keep_going = TRUE;
    while (keep_going)
    {
        
        SP_DEVINFO_DATA device_info_data;
        SP_DEVICE_INTERFACE_DATA interface_data;

        ZeroMemory(&device_info_data, sizeof(SP_DEVINFO_DATA));
        device_info_data.cbSize = sizeof(SP_DEVINFO_DATA);

        ZeroMemory(&interface_data, sizeof(SP_DEVICE_INTERFACE_DATA));
        interface_data.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
        
        keep_going = SetupDiEnumDeviceInterfaces(
            device_info_set,
            NULL, //&device_info_data,
            &hid_guid,
            index,
            &interface_data
        );

        if(keep_going)
        {
            DWORD buffer_size = 0;

            SetupDiGetDeviceInterfaceDetail(
                device_info_set,
                &interface_data,
                NULL,
                0,
                &buffer_size,
                NULL
            );

            PSP_DEVICE_INTERFACE_DETAIL_DATA device_info = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(buffer_size);
            device_info->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

            SetupDiGetDeviceInterfaceDetail(
                device_info_set,
                &interface_data,
                device_info,
                buffer_size,
                &buffer_size,
                NULL
                ); 

            rwr_device_name.assign(device_info->DevicePath);
            wprintf(_T("Device: %s\n"), rwr_device_name.c_str());
            if (rwr_device_name.find(_T("vid_1337")) != 0 && rwr_device_name.find(_T("pid_beef")))
            {
                _tprintf(_T("Found the RWR!\n"));
                keep_going = FALSE;
            }
            
            free(device_info);

        }
        index++;
    }

    SetupDiDestroyDeviceInfoList(device_info_set);


    // now that we have the device string, open a handle
    HANDLE rwr_handle = CreateFile(
        rwr_device_name.c_str(),
        GENERIC_WRITE | GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL  | FILE_FLAG_OVERLAPPED,
        NULL
    );

    if (rwr_handle == NULL)
        print_error(GetLastError());
    
    DWORD bytes_written;
    OVERLAPPED overlapped{ 0 };
    overlapped.Offset = 0;
    overlapped.OffsetHigh = 0;

    struct report_wrapper report;
    memset(&report, 0, 65);
    
    uint8_t bit_count = 0; // one bit set per iteration
    uint8_t raw_count = 0; // increments by 1
    int8_t signed_count = 0;

    // create the TCP socket
    
    WSADATA wsa_data;
    
    // bind the TCP socket
    int port = 1337;
    if (argc > 1)
        port = atoi(argv[1]);

    std::string port_string = std::to_string(port);

    SOCKET client;
    
    int bytes_received = 0;

    if (WSAStartup(MAKEWORD(2, 2), &wsa_data))
    {
        print_error(GetLastError());
        goto end;
    }

    _tprintf(_T("Networking started.\n"));
    while (1)
    {
        SOCKET sock;
        struct addrinfo hints = { 0 }, *result = NULL;

        memset(&hints, 0, sizeof(hints));
        result = 0;
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_flags = AI_PASSIVE;

        int retval = getaddrinfo("127.0.0.1", port_string.c_str(), &hints, &result);
        if (retval)
        {
            _tprintf(_T("getaddrinfo() error: %s\n"), gai_strerror(retval));
            goto end;
        }

        sock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
        if (sock == INVALID_SOCKET)
        {
            print_error(GetLastError());
            goto end;
        }

        if (bind(sock, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR)
        {
            print_error(GetLastError());
            goto end;
        }

        freeaddrinfo(result);

        if (listen(sock, SOMAXCONN) == SOCKET_ERROR)
        {
            printf("WSAGetLastError: %d\n", WSAGetLastError());
            goto end;
        }

        _tprintf(_T("Waiting for connection...\n"));

        client = accept(sock, NULL, NULL);
        if (client == INVALID_SOCKET)
        {
            print_error(GetLastError());
            goto end;
        }

        closesocket(sock);

        printf("A client connected!\n");

        while (1)
        {
            // loop forever while a client is connected

            // get some data from the network
            memset(input, 0, sizeof(input));
            int bytes_received = recv(client, input, sizeof(input), 0);
            if (bytes_received > 0)
            {
                //printf("Received %d bytes.\n", bytes_received);
            }
            else if (bytes_received == 0)
            {
                printf("Closing!\n");
                break;
            }
            else
            {
                printf("WSAGetLastError: %d\n", WSAGetLastError());
                goto end;
            }

            printf("Received (raw): %s\n", input);
            

            rapidjson::Document d;
            d.Parse(input);
            if (d.HasParseError())
            {
                printf("\nError(offset %u): %s\n",
                    (unsigned)d.GetErrorOffset(),
                    rapidjson::GetParseError_En(d.GetParseError()));
                continue;
            }

            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            d.Accept(writer);
            //printf("Received: %s\n", buffer.GetString());

            rapidjson::Pointer pointer = rapidjson::Pointer("/Emitters");
            
            rapidjson::Value* emitters = pointer.Get(d);
            
            tracking_target main_target;
            memset(&main_target, 0, sizeof(tracking_target));

            main_target.priority = -1; // we won't use this as a default for now
            std::vector<tracking_target> other_targets;

            if (emitters != nullptr)
            {
                if (emitters->IsArray())
                {
                    for (rapidjson::SizeType i = 0; i < emitters->Size(); i++)
                    {
                        if (emitters[i].IsArray())
                        {
                            printf("Submember %d is an array with %d members.\n", i, emitters[i].Size());

                            for (rapidjson::SizeType j = 0; j < emitters[i].Size(); j++)
                            {
                                if (emitters[i][j].HasMember("Priority") && emitters[i][j].HasMember("Azimuth") && emitters[i][j].HasMember("SignalType") && emitters[i][j].HasMember("ID") && emitters[i][j].HasMember("Power"))
                                {
                                    double power = emitters[i][j]["Power"].GetDouble();
                                    double azimuth = emitters[i][j]["Azimuth"].GetDouble();
                                    double priority = emitters[i][j]["Priority"].GetDouble();
                                    double type = emitters[i][j]["Priority"].GetDouble();
                                    std::string signal_type = std::string(emitters[i][j]["SignalType"].GetString());

                                    printf("ID: %s\n", emitters[i][j]["ID"].GetString());
                                    printf("SignalType: %s\n", signal_type.c_str());
                                    printf("Power: %f\n", power);
                                    printf("Priority: %f\n", priority);
                                    printf("Azimuth: %f\n", azimuth);
                                    printf("Type: %f\n", type);

                                    tracking_target self;
                                    memset(&self, 0, sizeof(self));
                                    self.priority = priority;
                                    self.azimuth = azimuth;
                                    self.power = power;
                                    //self.type = type;

                                    if (signal_type.compare("scan") == 0)
                                        self.signal_type = SCAN;
                                    else if (signal_type.compare("track_while_scan") == 0)
                                        self.signal_type = SCAN_AND_TRACK;
                                    else if (signal_type.compare("missile_active_homing") == 0)
                                        self.signal_type = MISSILE_ACTIVE_HOMING;
                                    else
                                        self.signal_type = UNKNOWN;

                                    if (priority > main_target.priority)
                                    {
                                        // make the old primary target an other object
                                        if (main_target.priority > 0)
                                        {
                                            other_targets.push_back(main_target);
                                        }

                                        // new primary target
                                        main_target.priority = self.priority;
                                        main_target.azimuth = self.azimuth;
                                        main_target.type = self.type;
                                        main_target.power = self.power;
                                    }
                                    else
                                    {
                                        // add this to our other objects we're tracking
                                        other_targets.push_back(self);
                                    }
                                }
                            }
                        }
                        else
                        {
                            //printf("Submember %d is not an array. It is a %s\n", i, kTypeNames[emitters[i].GetType()]);
                        }
                    }
                }
                else
                {
                    printf("Emitters is not an array (?)");
                }
            }








            // Set Feature
            memset(&report, 0, sizeof(report));
            report.report_number = 9;
            report.report.UpdateMask = UPDATE_SIGNAL_STRENGTH;
            if (main_target.priority > 0)
                report.report.UpdateMask |= UPDATE_MAIN_DIRECTION;

            //report.report.MiscDriver = bit_count;
            
            // update other targets
            report.report.UpdateMask &= ~UPDATE_OTHER_DIRECTION;
            for (tracking_target t : other_targets)
            {
                report.report.UpdateMask |= UPDATE_OTHER_DIRECTION;
                report.report.OtherDirection |= remap_other(degrees_to_value((int)(t.azimuth * 57.2958))); // radians to degrees
            }
            
            // update main target
            report.report.UpdateMask &= ~UPDATE_MAIN_DIRECTION;
            int main_azimuth_degrees = (int)(main_target.azimuth * 57.2958); // radians to degrees
            if (main_azimuth_degrees > 100)
            {
                report.report.UpdateMask |= UPDATE_MISC;
                report.report.MiscDriver |= MAIN_RIGHT_LED;
            }
            else if (main_azimuth_degrees < -100)
            {
                report.report.UpdateMask |= UPDATE_MISC;
                report.report.MiscDriver |= MAIN_LEFT_LED;
            }
            else
                report.report.MainDirection = main_azimuth_degrees;
            report.report.SignalStrength = (uint8_t)(main_target.power * 0xff);
            printf("signal strength: %f -> %d\n", main_target.power, report.report.SignalStrength);

            // TODO: Update the radar type based on the main target

            printf("Main direction: %f -> %d -> %d\n", main_target.azimuth, (int)(main_target.azimuth * 57.2958), report.report.MainDirection);

            report.report.RadarType = raw_count % 8;
            //printf("report size: %d\n", sizeof(report));

            // the write is overlapped, so don't check for a successful return here.
            // At some point (probably never), the trick here is to check to make sure the
            // overlapped operation, but I don't want to do that now. Here's a note in
            // case someone else wants to do it.
            WriteFile(rwr_handle, &report, 65, &bytes_written, &overlapped);

            // sleep for a bit
            Sleep(10);

            if (bit_count == 0)
                bit_count = 1;
            else
                bit_count = bit_count << 1;

            raw_count++;
            signed_count++;
        }

        // once the client has disconnected, start listening again
    }
end:
    WSACleanup();
    CloseHandle(rwr_handle);

    return 0;

}