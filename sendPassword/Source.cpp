#include <windows.h>
#include <winuser.h>
#include <map>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>

void sendEnter() {
    INPUT inputs[2] = {};
    ZeroMemory(inputs, sizeof(inputs));

    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_RETURN;

    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = VK_RETURN;
    inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;

    SendInput(2, inputs, sizeof(INPUT));
}

void sendPassword() {
    std::map<std::string, WORD> keyMap;
    keyMap["Enter"] = 0x0D;
    keyMap["0"] = 0x30;    keyMap["1"] = 0x31;    keyMap["2"] = 0x32;    keyMap["3"] = 0x33;
    keyMap["4"] = 0x34;    keyMap["5"] = 0x35;    keyMap["6"] = 0x36;    keyMap["7"] = 0x37;
    keyMap["8"] = 0x38;    keyMap["9"] = 0x39;

    keyMap["A"] = 0x41;    keyMap["B"] = 0x42;    keyMap["C"] = 0x43;    keyMap["D"] = 0x44;
    keyMap["E"] = 0x45;    keyMap["F"] = 0x46;    keyMap["G"] = 0x47;    keyMap["H"] = 0x48;
    keyMap["I"] = 0x49;    keyMap["J"] = 0x4A;    keyMap["K"] = 0x4B;    keyMap["L"] = 0x4C;
    keyMap["M"] = 0x4D;    keyMap["N"] = 0x4E;    keyMap["O"] = 0x4F;    keyMap["P"] = 0x50;
    keyMap["Q"] = 0x51;    keyMap["R"] = 0x52;    keyMap["S"] = 0x53;    keyMap["T"] = 0x54;
    keyMap["U"] = 0x55;    keyMap["V"] = 0x56;    keyMap["W"] = 0x57;    keyMap["X"] = 0x58;
    keyMap["Y"] = 0x59;    keyMap["Z"] = 0x5A;

    //If you want to hardcode the password instead of reading it from the file, define it here and comment out the lines 42-48.
    std::string password = "PaSsWoRd";
    
    //Read the password from file.
    std::ifstream passFile("password.txt");
    if (!passFile.is_open()) {
        std::cerr << "Failed to open password file." << std::endl;
        return;
    }
    std::getline(passFile, password);
    passFile.close();
    
    //Note here: I've tried many ways to send uppercase inputs but for the reasons I don't know a lot of them didn't work.
    //One works on Windows but magically doesn't work on Windows Server and only sends "@" as inputs etc.
    //Turning capslock on and off and waiting a bit seems the one it works normally.

    //If capslock is on, turn it off.
    if ((GetKeyState(VK_CAPITAL) & 0x0001) != 0) {
        INPUT capsInputs[2] = {};
        ZeroMemory(capsInputs, sizeof(capsInputs));

        capsInputs[0].type = INPUT_KEYBOARD;
        capsInputs[0].ki.wVk = VK_CAPITAL;

        capsInputs[1].type = INPUT_KEYBOARD;
        capsInputs[1].ki.wVk = VK_CAPITAL;
        capsInputs[1].ki.dwFlags = KEYEVENTF_KEYUP;

        SendInput(2, capsInputs, sizeof(INPUT));
    }

    //It works better when you wait a while before sending another input.
    Sleep(500);

    //Iterate through the password string.
    for (int i = 0; i < password.length(); i++) {
        std::string pasx(1, password[i]);
        //Check if uppercase.
        bool isUppercase = std::isupper(static_cast<unsigned char>(pasx[0]));
        //Get the uppercase to access from the map
        std::string keyVec(1, std::toupper(static_cast<unsigned char>(pasx[0])));

        //If uppercase and capslock is off, turn it on.
        if (isUppercase && ((GetKeyState(VK_CAPITAL) & 0x0001) == 0)) {
            INPUT capsInputs[2] = {};
            ZeroMemory(capsInputs, sizeof(capsInputs));

            capsInputs[0].type = INPUT_KEYBOARD;
            capsInputs[0].ki.wVk = VK_CAPITAL;

            capsInputs[1].type = INPUT_KEYBOARD;
            capsInputs[1].ki.wVk = VK_CAPITAL;
            capsInputs[1].ki.dwFlags = KEYEVENTF_KEYUP;

            SendInput(2, capsInputs, sizeof(INPUT));
        }

        //Send the input.
        INPUT keyInputs[2] = {};
        ZeroMemory(keyInputs, sizeof(keyInputs));

        keyInputs[0].type = INPUT_KEYBOARD;
        keyInputs[0].ki.wVk = keyMap[keyVec];

        keyInputs[1].type = INPUT_KEYBOARD;
        keyInputs[1].ki.wVk = keyMap[keyVec];
        keyInputs[1].ki.dwFlags = KEYEVENTF_KEYUP;

        SendInput(2, keyInputs, sizeof(INPUT));
    
        //Turn off the capslock if its on.
        if ((GetKeyState(VK_CAPITAL) & 0x0001) != 0) {
            INPUT capsInputs[2] = {};
            ZeroMemory(capsInputs, sizeof(capsInputs));

            capsInputs[0].type = INPUT_KEYBOARD;
            capsInputs[0].ki.wVk = VK_CAPITAL;

            capsInputs[1].type = INPUT_KEYBOARD;
            capsInputs[1].ki.wVk = VK_CAPITAL;
            capsInputs[1].ki.dwFlags = KEYEVENTF_KEYUP;

            SendInput(2, capsInputs, sizeof(INPUT));
        }
    }

    //Turn off capslock
    if ((GetKeyState(VK_CAPITAL) & 0x0001) != 0) {
        INPUT capsInputs[2] = {};
        ZeroMemory(capsInputs, sizeof(capsInputs));

        capsInputs[0].type = INPUT_KEYBOARD;
        capsInputs[0].ki.wVk = VK_CAPITAL;

        capsInputs[1].type = INPUT_KEYBOARD;
        capsInputs[1].ki.wVk = VK_CAPITAL;
        capsInputs[1].ki.dwFlags = KEYEVENTF_KEYUP;

        SendInput(2, capsInputs, sizeof(INPUT));
    }
}


int main(int argc, char* argv[])
{
    Sleep(200);
    sendPassword();
    sendEnter();
    Sleep(200);
    return 0;

}