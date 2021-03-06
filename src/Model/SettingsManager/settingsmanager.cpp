﻿// This file is part of the Silent.
// Copyright Aleksandr "Flone" Tretyakov (github.com/Flone-dnb).
// Licensed under the ZLib license.
// Refer to the LICENSE file included.

#include "settingsmanager.h"


// STL
#include <fstream>
#include <shlobj.h>

// Custom
#include "View/MainWindow/mainwindow.h"
#include "Model/SettingsManager/SettingsFile.h"


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------


SettingsManager::SettingsManager(MainWindow* pMainWindow)
{
    this ->pMainWindow   = pMainWindow;


    bSettingsFileCreatedFirstTime = false;


    pCurrentSettingsFile = nullptr;
    pCurrentSettingsFile = readSettings();
}





void SettingsManager::saveSettings(SettingsFile* pSettingsFile, bool bSetOnlyConnectInfo)
{
    // Get the path to the Documents folder.

    TCHAR   my_documents[MAX_PATH];
    HRESULT result = SHGetFolderPathW( nullptr, CSIDL_PERSONAL, nullptr, SHGFP_TYPE_CURRENT, my_documents );

    if (result != S_OK)
    {
        pMainWindow ->showMessageBox(true, "Can't open the Documents folder to read the settings.");

        delete pSettingsFile;

        return;
    }




    // Open the settings file.

    std::wstring sPathToOldSettings  = std::wstring(my_documents);
    std::wstring sPathToNewSettings  = std::wstring(my_documents);
    sPathToOldSettings  += L"\\" + std::wstring(SETTINGS_NAME);
    sPathToNewSettings  += L"\\" + std::wstring(SETTINGS_NAME) + L"~";

    std::ifstream settingsFile (sPathToOldSettings, std::ios::binary);
    std::ofstream newSettingsFile;

    if ( settingsFile .is_open() )
    {
        newSettingsFile .open(sPathToNewSettings);
    }
    else
    {
        newSettingsFile .open(sPathToOldSettings);
    }




    if ( bSetOnlyConnectInfo )
    {
        pSettingsFile ->iPushToTalkButton    = pCurrentSettingsFile ->iPushToTalkButton;
        pSettingsFile ->iMasterVolume        = pCurrentSettingsFile ->iMasterVolume;
        pSettingsFile ->sThemeName           = pCurrentSettingsFile ->sThemeName;
        pSettingsFile ->bPlayPushToTalkSound = pCurrentSettingsFile ->bPlayPushToTalkSound;
        pSettingsFile ->sInputDeviceName     = pCurrentSettingsFile ->sInputDeviceName;
    }
    else
    {
        pSettingsFile ->sConnectString = pCurrentSettingsFile ->sConnectString;
        pSettingsFile ->iPort          = pCurrentSettingsFile ->iPort;
        pSettingsFile ->sPassword      = pCurrentSettingsFile ->sPassword;
    }




    // Write new setting to the new file.

    // Push-to-Talk button
    newSettingsFile .write
            ( reinterpret_cast <char*> (&pSettingsFile ->iPushToTalkButton), sizeof(pSettingsFile ->iPushToTalkButton) );

    // Master Volume
    newSettingsFile .write
            ( reinterpret_cast <char*> (&pSettingsFile ->iMasterVolume),     sizeof(pSettingsFile ->iMasterVolume)     );




    if ( bSetOnlyConnectInfo )
    {
        char cUserNameLength = static_cast <char> ( pSettingsFile ->sUsername .size() );

        newSettingsFile .write
                ( reinterpret_cast <char*> (&cUserNameLength),          sizeof(cUserNameLength) );

        newSettingsFile .write
                ( pSettingsFile ->sUsername .c_str(), cUserNameLength );
    }
    else
    {
        if ( settingsFile .is_open() )
        {
            // Open the old file to copy user name.

            int iOldFileSize = 0;

            settingsFile .seekg(0, std::ios::end);
            iOldFileSize     = static_cast <int> ( settingsFile .tellg() );
            settingsFile .seekg( sizeof(pSettingsFile ->iPushToTalkButton) + sizeof(pSettingsFile ->iMasterVolume) );



            iOldFileSize    -= ( sizeof(pSettingsFile ->iPushToTalkButton)
                                 + sizeof(pSettingsFile ->iMasterVolume) );




            // Copy user name

            char cUserNameLength = 0;
            settingsFile .read ( reinterpret_cast <char*> (&cUserNameLength), sizeof(cUserNameLength) );

            char vBuffer[UCHAR_MAX];
            memset(vBuffer, 0, UCHAR_MAX);

            settingsFile .read( vBuffer, cUserNameLength );

            pSettingsFile ->sUsername = vBuffer;




            // Write this user name

            newSettingsFile .write
                    ( reinterpret_cast <char*> (&cUserNameLength),          sizeof(cUserNameLength) );

            newSettingsFile .write
                    ( pSettingsFile ->sUsername .c_str(), cUserNameLength );
        }
        else
        {
            // User name: no name
            unsigned char cNameSize = 0;
            newSettingsFile .write( reinterpret_cast <char*> (&cNameSize), sizeof(cNameSize) );
        }
    }


    // Theme name
    unsigned char cThemeLength = static_cast <unsigned char> ( pSettingsFile ->sThemeName .size() );

    newSettingsFile .write
            ( reinterpret_cast <char*> (&cThemeLength),          sizeof(cThemeLength) );

    newSettingsFile .write
            ( pSettingsFile ->sThemeName .c_str(), cThemeLength );



    // Push-to-Talk press/unpress sound
    char cPlayPushToTalkSound = pSettingsFile ->bPlayPushToTalkSound;

    newSettingsFile .write
            ( &cPlayPushToTalkSound, sizeof(cPlayPushToTalkSound) );


    // Connect string
    unsigned char cConnectStringSize = static_cast <unsigned char>( pSettingsFile ->sConnectString.size() );

    newSettingsFile .write
            ( reinterpret_cast <char*> (&cConnectStringSize), sizeof(cConnectStringSize) );

    newSettingsFile .write
            ( const_cast <char*>       (pSettingsFile ->sConnectString .c_str()), cConnectStringSize );


    // Port
    newSettingsFile .write
            ( reinterpret_cast <char*> (&pSettingsFile ->iPort), sizeof(pSettingsFile ->iPort) );


    // Password
    unsigned char cPassSize = static_cast <unsigned char>( pSettingsFile ->sPassword.size() );

    newSettingsFile .write
            ( reinterpret_cast <char*> (&cPassSize), sizeof(cPassSize) );

    newSettingsFile .write
            ( reinterpret_cast <char*>( const_cast<wchar_t*>(pSettingsFile ->sPassword .c_str()) ), cPassSize * 2 );


    // Input device
    unsigned char cInputDeviceNameSize = static_cast <unsigned char>( pSettingsFile ->sInputDeviceName.size() );

    newSettingsFile .write
            ( reinterpret_cast <char*> (&cInputDeviceNameSize), sizeof(cInputDeviceNameSize) );

    newSettingsFile .write
            ( reinterpret_cast <char*>( const_cast<wchar_t*>(pSettingsFile ->sInputDeviceName .c_str()) ), cInputDeviceNameSize * 2 );

    // NEW SETTINGS GO HERE
    // + don't forget to update "if ( bSetOnlyConnectInfo )" above, where code is:
    // "pSettingsFile ->iPushToTalkButton    = pCurrentSettingsFile ->iPushToTalkButton;"
    // + don't forget to update "readSettings()"



    if ( settingsFile .is_open() )
    {
        // Close files and rename the new file.

        settingsFile    .close();
        newSettingsFile .close();

        _wremove( sPathToOldSettings .c_str() );

        _wrename( sPathToNewSettings .c_str(), sPathToOldSettings .c_str() );
    }
    else
    {
        newSettingsFile .close();
    }




    // Save the old theme

    std::string sOldTheme = "";

    if (pCurrentSettingsFile)
    {
        sOldTheme = pCurrentSettingsFile ->sThemeName;
    }
    else
    {
        sOldTheme = pSettingsFile ->sThemeName;
    }

    // Update our 'pCurrentSettingsFile' to new settings

    // Careful here
    // AudioService takes settings (calls getSettings()) very often.

    mtxRefreshSettings .lock();


    if (pCurrentSettingsFile)
    {
        delete pCurrentSettingsFile;
        pCurrentSettingsFile = nullptr;
    }


    pCurrentSettingsFile = pSettingsFile;

    mtxRefreshSettings .unlock();




    // Update theme if it was changed

    if ( pCurrentSettingsFile ->sThemeName != sOldTheme )
    {
        pMainWindow ->applyTheme();
    }
}


SettingsFile *SettingsManager::readSettings()
{
    // Get the path to the Documents folder.

    TCHAR   my_documents[MAX_PATH];
    HRESULT result = SHGetFolderPathW( nullptr, CSIDL_PERSONAL, nullptr, SHGFP_TYPE_CURRENT, my_documents );

    if (result != S_OK)
    {
        pMainWindow ->showMessageBox(true, "Can't open the Documents folder to read the settings.");

        return nullptr;
    }




    // Delete FChatSettings (FChat - old version) if they are exists
    std::wstring adressToOldSettings = std::wstring(my_documents);
    adressToOldSettings += L"\\FChatSettings.data";

    std::ifstream oldSettings (adressToOldSettings);
    if (oldSettings .is_open())
    {
        oldSettings .close();
        _wremove ( adressToOldSettings .c_str() );
    }




    // Create settings file object

    SettingsFile* pSettingsFile = new SettingsFile();





    // Open the settings file.

    std::wstring adressToSettings = std::wstring(my_documents);
    adressToSettings             += L"\\" + std::wstring(SETTINGS_NAME);

    std::ifstream settingsFile (adressToSettings, std::ios::binary);

    if ( settingsFile .is_open() )
    {
        // Read the settings.



        // Read push-to-talk button
        settingsFile .read( reinterpret_cast <char*> (&pSettingsFile ->iPushToTalkButton), sizeof(pSettingsFile ->iPushToTalkButton) );



        // Read master volume
        settingsFile .read( reinterpret_cast <char*> (&pSettingsFile ->iMasterVolume),     sizeof(pSettingsFile ->iMasterVolume) );



        // Old version settings may end somewhere here
        if (settingsFile .eof())
        {
            settingsFile .close();

            return pSettingsFile;
        }



        // Read user name length
        unsigned char cUserNameLength = 0;
        settingsFile .read( reinterpret_cast <char*> (&cUserNameLength), sizeof(cUserNameLength) );

        // Read user name
        char vBuffer[UCHAR_MAX];
        memset(vBuffer, 0, UCHAR_MAX);

        settingsFile .read( vBuffer, cUserNameLength );

        pSettingsFile ->sUsername = vBuffer;



        // Read theme length
        unsigned char cThemeLength = 0;
        settingsFile .read( reinterpret_cast <char*> (&cThemeLength), sizeof(cThemeLength) );

        // Read theme name
        memset(vBuffer, 0, UCHAR_MAX);

        settingsFile .read( vBuffer, cThemeLength );

        pSettingsFile ->sThemeName = vBuffer;




        // Old version settings may end somewhere here
        if (settingsFile .eof())
        {
            settingsFile .close();

            return pSettingsFile;
        }



        // Read push-to-talk sound enabled
        char cPushToTalkSoundEnabled = 0;
        settingsFile .read( &cPushToTalkSoundEnabled, sizeof(cPushToTalkSoundEnabled) );

        pSettingsFile ->bPlayPushToTalkSound = cPushToTalkSoundEnabled;



        if (settingsFile .eof())
        {
            settingsFile .close();

            return pSettingsFile;
        }


        // Read connect string
        unsigned char cConnectStringSize = 0;
        settingsFile .read( reinterpret_cast<char*>(&cConnectStringSize), sizeof(cConnectStringSize) );

        memset(vBuffer, 0, UCHAR_MAX);
        settingsFile .read(vBuffer, cConnectStringSize);

        pSettingsFile ->sConnectString = vBuffer;


        // Read port
        settingsFile .read( reinterpret_cast<char*>(&pSettingsFile ->iPort), sizeof(pSettingsFile ->iPort));


        // Read password
        unsigned char cPasswordSize = 0;
        settingsFile .read( reinterpret_cast<char*>(&cPasswordSize), sizeof(cPasswordSize) );

        wchar_t vWBuffer[UCHAR_MAX];
        memset(vWBuffer, 0, UCHAR_MAX * sizeof(wchar_t));

        settingsFile .read(reinterpret_cast<char*>(vWBuffer), cPasswordSize * sizeof(wchar_t));

        pSettingsFile ->sPassword = vWBuffer;


        if (settingsFile .eof())
        {
            settingsFile .close();

            return pSettingsFile;
        }


        // Input device
        unsigned char cInputDeviceSize = 0;
        settingsFile .read( reinterpret_cast<char*>(&cInputDeviceSize), sizeof(cInputDeviceSize) );

        wchar_t vDeviceBuffer[UCHAR_MAX];
        memset(vDeviceBuffer, 0, UCHAR_MAX * sizeof(wchar_t));

        settingsFile .read(reinterpret_cast<char*>(vDeviceBuffer), cInputDeviceSize * sizeof(wchar_t));

        pSettingsFile ->sInputDeviceName = vDeviceBuffer;


        settingsFile .close();
    }
    else
    {
        // The settings file does not exist.
        // Create one and write default settings.


        bSettingsFileCreatedFirstTime = true;
    }



    return pSettingsFile;
}

SettingsFile *SettingsManager::getCurrentSettings()
{
    mtxRefreshSettings .lock();
    mtxRefreshSettings .unlock();

    return pCurrentSettingsFile;
}

bool SettingsManager::isSettingsCreatedFirstTime()
{
    return bSettingsFileCreatedFirstTime;
}

SettingsManager::~SettingsManager()
{
    if (pCurrentSettingsFile)
    {
        delete pCurrentSettingsFile;
    }
}
