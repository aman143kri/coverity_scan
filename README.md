# BIBIFI Project
CMPT 785 BIBIFI project.

## Dependencies
This project makes use of the following dependencies:
1. nhlohmann json
2. OpenSSL for C++ dev

## Notes
Our executable will generate two directores on the same level as the executable. One will be "filesys" and the other will be "keyfiles". The keyfiles model the external location of storage usually accessed through a network. 

When running the program for the first time, no keyfiles are needed. 

After running the first time, an Admin keyfile will be generated. To log in using that keyfile, the user simply has to pass the username (associated with the keyfile) to the executable. For instance, 

./executable Admin

to log in as Admin.

The shared directory behaves a differently from regular directories. Because of our implementation, when users need to cat files, they will need to pass a second number argument to denote which file they want to print out. The shared directory is owned by the system. All files listed in the shared directory are symbolic in nature. No files can be created by the user in shared directory.

For instance, cat file.txt 3 will cat the third in the list shown by ls.

The Admin user has extra privileges, and they will be able to get a view of the entire filesystem, including "out of bound" areas for regular users. However, they cannot touch any system files - only traverse it.

When using mkfile, we intentionally expect the a string to be wrapped in "". The beginning and ending " will mark the starting and ending point of the argument.

Our filesystem uses a access-by-permission system. Each directory and file can be granted read/cd access to other users, known as collaborators. Permissions are not recursive or retroactive to parents, so if a specific directory is granted cd access to a user, they will only be able to visit that directory. Similarly, users can only see items that have been shared with them. Owners do not need to share the entire path to allow access to a specific directory, but if they want the path to be visible to the user being shared to, they will need to share each parent directory along the path.


