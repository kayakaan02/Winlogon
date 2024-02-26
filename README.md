# Winlogon
Type your password in Windows login screen


First things first, there is two Visual Studio projects you should compile.

`sendPassword` is about typing the password
and `createProcess` is about providing the required permissons.

I wrote those long time ago so I don't remember every detail but you should read the rest before using it.

#`sendPassword`:
- Normally the code gets the password from "password.txt" file. But if you want to hardcode it, define it in the line 39 and comment out the lines 42-48.



#`createProcess`:
- This is the tricky part, You have to install this project's executable as a service in order to get the required permissions. Then it creates the       
  `sendPassword` project's executable. As I remember set the service to stop when the executable ends/killed. Then you can type `net start {service name}` in   
  terminal to start it.

- In the line 187. set the directory and the `sendPassword.exe`'s name if you want. In the code it looks like this:
  `CreateProcessAsUserA(hToken, "C:\\sendin\\sendPassword.exe", NULL, NULL, NULL, bInheritHandles, dwCreationFlags, lpEnvironment, "C:\\sendin\\", &si, &pi)`
  change ` "C:\\sendin\\sendPassword.exe"` to `"{directory}\\{executable}.exe"`, and `"C:\\sendin\\"` to `"{directory}"`.

- It tries to log in 6 times, and checks if the wrong password is entered. If you want you can disable it that was for problematic Windows Server behaviour.
  check the lines 184, 206 and the function `isSessionLocked()`.

  Thats it.
