#include <Windows.h>
#include <hidsdi.h>
#include <hidclass.h>
#include <setupapi.h>
#include <tchar.h>
#include <string>
#include <stdio.h>

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

#pragma pack(push,1)
struct report_wrapper {
    BYTE report_number;
    USB_JoystickReport_Data_t report;
};
#pragma pack(pop)

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

    while (1)
    {
        // Set Feature
        memset(&report, 0, sizeof(report));
        report.report_number = 9;
        report.report.UpdateMask = UPDATE_OTHER_DIRECTION | UPDATE_MAIN_DIRECTION | UPDATE_RADAR_TYPE | UPDATE_MISC | UPDATE_SIGNAL_STRENGTH;
        report.report.MiscDriver = bit_count;
        report.report.OtherDirection = remap_other(degrees_to_value(signed_count));
        report.report.MainDirection = -signed_count;
        report.report.SignalStrength = raw_count;
        report.report.RadarType = raw_count % 8;
        //printf("report size: %d\n", sizeof(report));
        if (!WriteFile(rwr_handle, &report, 65, &bytes_written, &overlapped))
        {
            int error = GetLastError();
            _tprintf(_T("ERROR_IO_PENDING : %d\n"), ERROR_IO_PENDING);
            _tprintf(_T("error : %d\n"), error);

            print_error(error);
        }

        printf("raw_count: %d\n", raw_count);

        // sleep for a bit
        Sleep(10);

        if (bit_count == 0)
            bit_count = 1;
        else
            bit_count = bit_count << 1;

        raw_count++;
        signed_count++;
    }


    /*
    report[0] = 0xFF;
    report[1] = UPDATE_RADAR_TYPE;
    report[2] = AIRBORNE;
    report[3] = LONG_RANGE;
    report[4] = MEDIUM_RANGE;
    report[5] = SHORT_RANGE;
    report[6] = EARLY_WARNING;
    report[7] = AWACS;
    */
    
    
    CloseHandle(rwr_handle);

    return 0;

}