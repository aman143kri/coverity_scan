/*	ShellCommands.cpp
 *
 *	Source code organizational unit for all functions that deals with user functionality.
 *
 */

#include "../headers/modules.h"
#include "../headers/cipher.h"
#include <cctype> // for isalnum()

void cd(std::string unsanitized_target_path)
{
    // first sanitize path and make sure the destination exists
	bool path_changed = false;
    bool proper_path = false;
    //printf("[cd] Calling SanitizePath() with input %s to check.\n", unsanitized_target_path.c_str());
    std::string target_path = SanitizePath(unsanitized_target_path, &proper_path);
    //printf("[cd] Sanitized path is %s.\nPerforming SystemTraversal() next.", target_path.c_str());
    njson *target = SystemTraversal(SubtractSubstring(target_path, base_anchor_path));
    // TODO: I may want to check proper path before pasting into SystemTraversal,
    //  but since I want to check that SystemTraversal is behaving itself, I'll keep it
    //  separate for now.

    //printf("[cd] Finished retrieving target json. Null? %s\n", target == nullptr ? "yes" : "no");
    if (target != nullptr and proper_path)
    {
        //printf("[cd] Target json is not a nullpointer and is checked to be a proper path.\n");
        // make sure that the current user is trying to cd into a valid directory
        if (DirectoryCheck(target) and VerifyLayer(target))
        {
            // make sure that the current user has permission to access this directory
            bool rpermissions = PermissionCheck(target, session_username);
            bool wpermissions = OwnerCheck(target, session_username);
            //printf("[cd] rpermissions: %s, wpermissions: %s\n", rpermissions ? "true" : "false", wpermissions ? "true" : "false");
            if (rpermissions or wpermissions)
            {
                //printf("[cd] Valid target, changing directories...\n");
                dir_ptr = target_path;
				path_changed = true;
            }
		}

    }

	if(not path_changed) {
		printf("Unable to change into the provided directory.\n");
	}


    //printf("[cd] Currently point at %s\n", dir_ptr.c_str());
}

void pwd()
{
    std::string display_path;
    std::string dir_ptr_for_pwd;

    // for the admin at root case: takes care of the dir_ptr initiation problem: add / at the end of dir_ptr if missing
    if (dir_ptr.rfind('/') != dir_ptr.length() - 1)
    {
        dir_ptr_for_pwd = dir_ptr + '/';
    }
    else
    {
        dir_ptr_for_pwd = dir_ptr;
    }
    // Check if the dir_ptr contains the base_path at the start (as it should)
    if (dir_ptr_for_pwd.rfind(base_path, 0) == 0)
    {
        //std::cout << "[pwd] dir_ptr for pwd: " << dir_ptr_for_pwd << std::endl;
        //std::cout << "[pwd] base_path: " << base_path << std::endl;

        // strip the base_path from dir_ptr_for_pwd for display.
        display_path = '/' + dir_ptr_for_pwd.substr(base_path.length());
    }
    else
    {
        // Handling the unexpected case where dir_ptr_for_pwd doesn't start with base_path
        //std::cout << "[pwd] dir_ptr: " << dir_ptr << std::endl;
        //std::cout << "[pwd] dir_ptr_for_pwd: " << dir_ptr_for_pwd << std::endl;
        //std::cout << "[pwd] base_path: " << base_path << std::endl;
        std::cerr << "Error: Unexpected path format. The current directory path does not begin with the base path." << std::endl;
        return;
    }

    // Print the final path
    std::cout << display_path << std::endl;
}

// helper for ls()
std::vector<std::string> GetSharedItems()
{
    std::vector<std::string> sharedItems;
    ifstream jfile(filesys::path(base_anchor_path + "filesys/metadata/shared.json"));
    // Parse the JSON file
    njson sharedJson;
    jfile >> sharedJson;
    if (jfile.fail())
    {
        std::cerr << "Parsing shared.json error." << std::endl;
        return sharedItems; // Return the empty vector
    }
    // Close the file after parsing the JSON
    jfile.close();

    // get the shrObjects' filenames under current user
    if (sharedJson.contains(session_username)){
        for (const auto &item : sharedJson[session_username])
        {
            string name = item["fileName"].get<std::string>();
            //std::cout << name << std::endl;
            sharedItems.push_back(name);
        }
    }
    else
    {
        // Handle the case where session_username doesn't exist or has an empty array.
        std::cerr << "No shared items found for user." << std::endl;
    }

    return sharedItems;
}

void ls()
{
    std::vector<std::string> itemsToDisplay;

    // Retrieve the current directory object using Dir ptr
    // std::cout << "[ls] dir_ptr: " << dir_ptr << std::endl;
    std::string dir_ptr_path_for_SystemTraversal = dir_ptr.substr(base_anchor_path.length());
    // std::cout << "[ls] dir_ptr path for SystemTraversal: " << dir_ptr_path_for_SystemTraversal << std::endl;
    njson *current_obj_ptr = SystemTraversal(dir_ptr_path_for_SystemTraversal);

    // in shared directory
    if ((*current_obj_ptr)["objType"] == "sdirectory")
    {
        //std::cout << "[ls] starting to get shared items" << std::endl;
        itemsToDisplay = GetSharedItems();
    }
    else
    {
        // not in shared directory
        int childLen = (*current_obj_ptr).at("objChildren").size();

        for (int i = 0; i < childLen; i++)
        {
            njson &child = (*current_obj_ptr).at("objChildren").at(i);
            njson *childPtr = &child;
            if (OwnerCheck(childPtr, session_username) || PermissionCheck(childPtr, session_username))
            {
                std::string childName = child["objName"];
                if (child["objType"] == "directory" or child["objType"] == "sdirectory")
                {
                    childName = "d -> " + childName + "/";
                }
                else if (child["objType"] == "file")
                {
                    childName = "f -> " + childName;
                }

                itemsToDisplay.push_back(childName);
            }
        }
    }

    //std::cout << "[ls] Print the items to display:" << std::endl;
    std::cout << "d -> ." << std::endl;
    std::cout << "d -> .." << std::endl;
    for (const auto &item : itemsToDisplay)
    {
        std::cout << item << std::endl;
    }
}

// helper for adduser()
// at first list of banned words
const std::vector<std::string> bannedWords = {"System", "Admin"};
bool IsValidUsername(const std::string &username)
{
    // Check if the username is in the list of banned words
    //std::cout << "[IsValidUsername] validating username....\n";
    if (std::find(bannedWords.begin(), bannedWords.end(), username) != bannedWords.end())
    {

        std::cout << "Username is not allowed. Please try again." << std::endl;
        return false;
    }

    // Check if the username already exists in the keymapper
    njson sys_keymapper = sys_load_keymapper();
    if (sys_keymapper.contains(username))
    {
        std::cout << "Username already exists. Please try again." << std::endl;
        return false;
    }

    // Check if the username is not empty
    if (username.empty())
    {
        std::cout << "Invalid username provided.\n";
        return false;
    }

    for (char c : username)
    {
        if (!std::isalnum(c))
        {
            std::cerr << "Invalid username. Only alphanumeric characters are allowed." << std::endl;
            return false;
        }
    }

    //std::cout << "Username is valid." << endl;
    return true;
}

void adduser(std::vector<std::string> commands)
{
    std::string username = commands[1];
    //printf("[adduser] We haven't crashed yet!");
    if (session_username != "Admin")
    {
        std::cout << "The current user does not have permissions to create users." << std::endl;
        return;
    }
    bool validUsernameEntered = IsValidUsername(username);
    if (!validUsernameEntered)
    {
        return;
    }
    else
    {
        AdminCreateUser(username);
    }
}

void cat2(vector<string> args) {

	/*
		Goal of this module:
		First, take the user input and sanitize it so that we have an absolute path.
		From there, we can extract the object parent and figure out whether it is 
		a directory or sdirectory.

		For instance, filesys/origin/root/user1/file.txt

		We will be looking for the parent user1/

		Based on that, we can decide whether user1/ is "sdirectory" or "directory"
		Depending on the objType, we must adjust how cat behaves.

		In the regular directory case, we can take a look at the child object file.txt
		and then check the collaborators. If the session_username is part of the collab,
		they have permissions to cat and so we will decrypt the contents and display it.

		However, in the sdirectory case, we will need to retrieve the shared.json contents
		and grab the list of files from there. We then check whether the desired file
		is in this list, and if it is, we can load it based on the registered path and
		print out the contents.
	*/

	string filename = args[1];
	bool proper_path = false;
	string sanitized_path = SanitizePath(filename, &proper_path);
	string sanitized_parent = sanitized_path;

	// we'll obtain the parent now
	if(sanitized_path.back()=='/') {
		sanitized_path.pop_back();
	}

	// cut away the child object in the path
	sanitized_parent = sanitized_parent.substr(0, sanitized_parent.find_last_of("/"));

	//printf("[cat2] The sanitized path is %s\n", sanitized_path.c_str());
	//printf("[cat2] The sanitized parent is %s\n", sanitized_parent.c_str());

	// obtained the object parent
	njson * objParent = SystemTraversal(SubtractSubstring(sanitized_parent, base_anchor_path));

	// we should only do things if the parent of the target file exists!
	if(objParent!=nullptr) {
		// we can now check for the type of this directory
		string objType = (*objParent)["objType"];
		//printf("[cat2] The object type of the parent is determined to be %s\n", objType.c_str());

		if(objType=="directory") {
			njson objChildren = (*objParent)["objChildren"];
			njson objChild = {};
			string target_child_name = SubtractSubstring(sanitized_path, sanitized_parent);
			target_child_name = target_child_name.substr(1, target_child_name.length());
			//printf("[cat2] The target child name is %s\n", target_child_name.c_str());

			// loop through the children array to find the target child
			for(const njson child:objChildren) {
				if(child["objName"]==target_child_name) {
					// found the child by name
					objChild = child;
					//printf("[cat2] We found the child!");
					break;
				}
			}

			// we can continue processing if the child is non-empty
			if(not objChild.empty()) {
				// now we can check for permissions to see if we can print the file
				bool is_collaborator = false;
				njson collaborators = objChild["objCollaborators"];
				string owner = objChild["objOwner"];
				for(const njson collaborator:collaborators) {
					// check if the current session user is part of the object's collaborators
					if(collaborator==session_username) {
						is_collaborator = true;
						//printf("[cat2] We found that %s is a collaborator.\n", session_username.c_str());
						break;
					}
				}

				bool is_owner = (owner==session_username);

				// if we're part of the collaborators, then we're good to print out details
				if(is_collaborator or is_owner) {
					// let's grab this file!
					ifstream infile(sanitized_path);
					string ciphertext="";

					if(infile.is_open()) {
						string line;
						while(getline(infile, line)) {
							ciphertext += line + "\n";
						}
						infile.close();
					}

					// next we need to decrypt the ciphertext.
    				njson keymap = sys_load_keymapper();
					try {
						// shouldn't happen, but in case someone deletes our keymap file...
						string ownerkey = keymap[owner];
						//printf("[cat2] The owner is %s with the key %s\n", owner.c_str(), ownerkey.c_str());

						// do the cryptography stuff
						Cipher decryptor;
						int cipher_len = ciphertext.length();
						string plaintext = decryptor.Decrypt(ciphertext, ownerkey, &cipher_len);

						// finally, print out the plaintext
						cout << plaintext << endl;

					} catch(const exception err) {
						cout << "Unable to display contents of " << filename << endl;
					}


				} else {
					cout << "Unable to display contents of " << filename << endl;
				}

			}

		} else if(objType=="sdirectory") {
			// we expect the user to see something like:
			// (1) filename
			// (2) filename
			// (3) filename
			// and they would cat filename n to print the n-th file
			// therefore, we take another arg filename n which describes which filename we want
			// to cat
			//printf("[cat2] Attempting to cat out from a symbolic directory!\n");	
			int file_num = -1;
			try {
				if(args.size()==3) {
					file_num = stoi(args[2]);
				}
			} catch(const exception err) {
				cout << "Invalid argument. Please enter a number as an argument." << endl;
			}

			//printf("[cat2] Do we have a segmentation fault?\n");

			// continue to process the command if valid file number
			if(file_num>0) {
				// load the shared.json items for the current user and check
				// if the current cat file name is part of the items.
				njson shared_objects = GetSharedObjects();

				//cout << "[cat2] file_num " << file_num << endl;
				//cout << "[cat2] shared_objects.size()" << shared_objects.size() << endl;

				if(file_num<=shared_objects.size()) {

					//printf("[cat2] Attempting to retrieve item at index %d\n", file_num);
					//njson target_object = shared_objects[file_num];
					njson target_object = shared_objects[file_num-1];
					//printf("[cat2] Successfully retrieved the object.\n");

					string target_path = "";
				
					//cout << "[cat2] target object is " << target_object.dump() << endl;
	
					// check if the object name matches the target filename
					string target_child_name = SubtractSubstring(sanitized_path, sanitized_parent);
					target_child_name = target_child_name.substr(1, target_child_name.length());

					if((target_object["fileName"])==target_child_name) {
						target_path = target_object["filePath"];
					}

					//printf("[cat2] The target path is %s\n", target_path.c_str());

					// once we have the file path, we can retrieve the systemObject related to it
					// and check for collaborator permissions
					njson * target_sysObject = SystemTraversal(SubtractSubstring(target_path, base_anchor_path));
					
					if(target_sysObject!=nullptr) {
						// shouldn't be null, but just in case...
						bool owner_perms = OwnerCheck(target_sysObject, session_username);
						bool collab_perms = PermissionCheck(target_sysObject, session_username);
						bool contained = VerifyLayer(target_sysObject);

						//printf("[cat2] Setting up the owner");

						string owner = (*target_sysObject)["objOwner"];

						if(contained and (owner_perms or collab_perms)) {
							
							// let's grab this file!
							ifstream infile(target_path);
							string ciphertext = "";
							
							if(infile.is_open()) {
								string line;
								while(getline(infile, line)) {
									ciphertext += line + "\n";
								}
								// close file after it is done
								infile.close();
							}	

							// next we need to decrypt the ciphertext.
							njson keymap = sys_load_keymapper();
							try {
								// shouldn't happen, but in case someone deletes our keymap file...
								string ownerkey = keymap[owner];
								//printf("[cat2] The owner is %s with the key %s\n", owner.c_str(), ownerkey.c_str());

								// do the cryptography stuff
								Cipher decryptor;
								int cipher_len = ciphertext.length();
								string plaintext = decryptor.Decrypt(ciphertext, ownerkey, &cipher_len);

								// finally, print out the plaintext
								cout << plaintext << endl;

							} catch(const exception err) {
								cout << "Unable to display contents of " << filename << endl;
							}


						}

					}

				}

			} else {
				cout << "cat with symbolic files expect an additional position argument." << endl;
			}

		}

	}	
	
}


// helper for getting shared objects for the current user
njson GetSharedObjects()
{
    njson sharedObjects = njson::array();
    ifstream jfile(filesys::path(base_anchor_path + "filesys/metadata/shared.json"));
    // Parse the JSON file
    njson sharedJson;
    jfile >> sharedJson;
    if (jfile.fail())
    {
        std::cerr << "Parsing shared.json error." << std::endl;
        return sharedObjects; // Return the empty vector
    }
    // Close the file after parsing the JSON
    jfile.close();

    // get the shrObjects' filenames under current user
    if (sharedJson.contains(session_username)){
		njson shrObjects = sharedJson[session_username];
		//cout << "[GetSharedObjects] Debugging: " << shrObjects.dump() << endl;
        for (const njson shrObject:shrObjects)
        {
			//cout << "[GetSharedObjects] Retrieving object" << shrObject.dump() << endl;
            sharedObjects.push_back(shrObject);
        }
    }
    else
    {
        // Handle the case where session_username doesn't exist or has an empty array.
        std::cerr << "No shared objects found for user." << std::endl;
    }

    return sharedObjects;
}


void cat(vector<string> args)
{
    string filename = args[1];
    bool proper_path = false;
    string path = SanitizePath(filename, &proper_path);
    njson *obj = SystemTraversal(SubtractSubstring(path, base_anchor_path));
    
    //  Validate that file exists.
    if (obj == nullptr)
    {
        cout << "File do not exist." << endl;
        return;
    }

    bool layer = VerifyLayer(obj);
    bool owner = OwnerCheck(obj, session_username);
    bool permission = PermissionCheck(obj, session_username);

    // Validate the layer and ownership
    if (!layer)
    {
        cout << "Can not perform cat on file:" << filename << endl;
        return;
    }
    // if is the owner or has permission perform cat
    if (!owner && !permission)
    {
        cout << "Not permission to open file '" << filename << "'" << endl;
        return;
    }

    ifstream file(filename);
    string fullfile;
    // ciphertext = B64.decode(file); according to documentation the exit of the encryption is a b64 string no need to convert that string again into a b64

    if (file.is_open())
    {
        string line;
        while (getline(file, line))
        {
            fullfile += line + "\n";
        }
        file.close();
    }
    else
    {
        cerr << "Error: Unable to open file '" << filename << "'" << endl;
    }
    njson keymap = sys_load_keymapper();
    int fullfilelength = fullfile.length();
    Cipher decription;
    if (!owner)
    {
        string owner_username= (*obj)["objOwner"];
        string plaintext = decription.Decrypt(fullfile, keymap[owner_username], &fullfilelength);
        cout << plaintext << endl;
        return;
    }

    string plaintext = decription.Decrypt(fullfile, keymap[session_username], &fullfilelength);
    cout << plaintext << endl;
}

void share(vector<string> args)
{
    string userinput = args[1];
    string username = args[2];
    bool proper_path = false;
    string path = SanitizePath(userinput, &proper_path);
    njson *obj = SystemTraversal(SubtractSubstring(path, base_anchor_path));

    // cout << "Content of the pointer: " << (*obj) << endl;
    //  validate if file exist
    if (obj == nullptr)
    {
        cout << "File or directory does not exist." << endl;
        return;
    }
    bool layer = VerifyLayer(obj);
    bool owner = OwnerCheck(obj, session_username);
    bool owner_user = OwnerCheck(obj, username);
    bool permissions = PermissionCheck(obj, username);
    //cout << "[share] owner_user  " << owner_user << endl;
    //cout << "[share] permissions " << permissions << endl;
    // validate layer and ownership
    if (!layer || !owner)
    {
        cout << "File or directory can not be shared." << endl;
        return;
    }
    // Validate if permissions
    if (permissions && !owner_user)
    {
        cout << "User" << username << "is already a collaborator." << endl;
        return;
    }
    // Validate that you are the owner using that username diff to sesion_username
    if (owner && username == session_username)
    {
        cout << "You are the owner of this file." << endl;
        return;
    }
    // Validate User exist
    njson keymap = sys_load_keymapper();
    if (!keymap.contains(username))
    {
        cout << "Unable to share the file with " << username << "." << endl;
        return;
    }

	string objType = (*obj)["objType"];
    bool added = false;
	if(objType=="file") {
		added = AddCollaborator(path, username);
	} else {
		//printf("[share] detected directory, calling UpdateSharedJsonAddCollab");
		added = UpdateSystemJsonAddCollab(path, username);
	}


    if (!added)
    {
        cout << username << " was not added to " << userinput << "as a collaborator." << endl;
    }
    else
    {
	/*
        string shared_username = "shared-" + username + "/";
        string temporal_path = SanitizePath(shared_username, &proper_path);

        // Small tokenizer to form the correct path
        vector<string> token;
        stringstream check1(temporal_path);
        string intermediate;

        while (getline(check1, intermediate, '/'))
        {
            token.push_back(intermediate);
            // cout << "[mkfile] token" << token << endl;
        }

        string path_username = "";
        for (int i = 0; i < token.size() - 2; i++)
        {
            path_username += token[i] + "/";
        }

        path_username += shared_username;

        njson *shared_obj = SystemTraversal(SubtractSubstring(path_username, base_anchor_path));
        (*shared_obj)["objChildren"].push_back(*obj);

        WriteChangeToSystemJsonFile();

        path_username += userinput;
        UpdateSharedJsonAddCollab(path_username, username);
		*/

        cout << username << " added to " << userinput << " as a collaborator." << endl;
    }
}

void mkfile(vector<string> args)
{
    string userinput = args[1];
    string plaintext = args[2];

	//printf("[mkfile] Used passed args %s, %s\n", userinput.c_str(), plaintext.c_str());

    bool proper_path = false;
    string file = SanitizePath(userinput, &proper_path);

	//printf("[mkfile] The sanitized path is %s\n", file.c_str());

    njson *objfile = SystemTraversal(SubtractSubstring(file, base_anchor_path));

    // Encript
    njson keymap = sys_load_keymapper();
    int plaintextlength = plaintext.length();
    Cipher encription;
	string tmp = keymap[session_username]; // tmp
	//printf("[mkfile] plaintext = %s, key = %s\n", plaintext.c_str(), tmp.c_str());
    string encryptedtext = encription.Encrypt(plaintext, keymap[session_username], &plaintextlength);

    // small tokenizer based on / for filename
    vector<string> token;
    stringstream check1(file);
    string intermediate;

    while (getline(check1, intermediate, '/'))
    {
        token.push_back(intermediate);
        // cout << "[mkfile] token" << token << endl;
    }
    string filename = token[token.size() - 1];

	//printf("[mkfile] The processed filename is %s\n", filename.c_str());

    ///////////////////// Case file already exists/////////////////////
    if (objfile != nullptr)
    {
		//printf("[mkfile] Case: file already exists and we are overwriting.\n");
        bool layer = VerifyLayer(objfile);
        bool owner = OwnerCheck(objfile, session_username);

        if (!layer || !owner)
        {
            cout << "No permission to write to file " << filename << "." << endl;

            return;
        }

		//printf("[mkfile] Now writing the file to location %s\n", file.c_str());

        ofstream encryptedfile(file.c_str());
        if (!encryptedfile.is_open())
        {
            cout << "Failed writing to file " << filename << "." <<endl;
            return;
        }

		encryptedfile << encryptedtext;
		encryptedfile.close();

		/*
		printf("[mkfile] Updating changes to our systemtree...\n");
		printf("[mkfile] The following are parameters:\n");
		printf("-> objName: %s\n", filename.c_str());	
		printf("-> objLevel: %d\n", (*objfile)["objLevel"].get<int>() + 1);

        njson change_file = {{"objName", filename},
                             {"objType", "file"},
                             {"objLevel", (*objfile)["objLevel"].get<int>() + 1},
                             {"objOwner", session_username},
                             {"objCollaborators", njson::array()},
                             {"objChildren", njson::array()}};

        change_file["objCollaborators"].push_back("Admin");
        (*objfile)["objChildren"].push_back(change_file);

		printf("[mkfile] Writing the changes to the system json.");
        WriteChangeToSystemJsonFile();
		*/

        // File created successfully
        encryptedfile.close();
        //cerr << "Success writing to file:" << filename << endl;
        return;
    }

    ///////////////////// Case file do not exists/////////////////////
    // use of the small tokenizer based on / for path

    string path = "";
    for (int i = 0; i < token.size() - 1; i++)
    {
        path += token[i] + "/";
    }

	//printf("[mkfile] Case where file does not exist.\n");
	//printf("[mkfile] The path is constructed from tokens is %s\n", path.c_str());
    njson *objpath = SystemTraversal(SubtractSubstring(path, base_anchor_path));

	//printf("[mkfile] We got the objpath, but do we worry about nullptr?, %s\n", (objpath==nullptr)?"true":"false");

	if(objpath==nullptr) {
		// in case the parent directory is a bad path, we will kill the execution
		cout << "Invalid path provided." << endl;
		return;
	}

    bool layer = VerifyLayer(objpath);
    bool owner = OwnerCheck(objpath, session_username);

    // Validate that you are the dir owner and correct layer
    if (!layer || !owner)
    {
        cout << "The current user does not have  permission to create the file " << filename << "." << endl;
        return;
    }

	//printf("[mkfile] Are we still dealing with sanitzed path? %s\n", file.c_str());
    ofstream encryptedfile(file.c_str());

    if (!encryptedfile.is_open())
    {
        cerr << "Failed to create the file." << endl;
        return;
    }

	encryptedfile << encryptedtext;

    // File created successfully
    encryptedfile.close();

	//printf("[mkfile] We're adding a new file so we need to update metadata.\n");
	//printf("-> objName: %s\n", filename.c_str());
	//printf("-> objLevel: %d\n", (*objpath)["objLevel"].get<int>() + 1);
    njson new_file = {{"objName", filename},
                      {"objType", "file"},
                      {"objLevel", (*objpath)["objLevel"].get<int>() + 1},
                      {"objOwner", session_username},
                      {"objCollaborators", njson::array()},
                      {"objChildren", njson::array()}};

    new_file["objCollaborators"].push_back("Admin");
    (*objpath)["objChildren"].push_back(new_file);


    AddCollaborator(file, "Admin");
    //WriteChangeToSystemJsonFile();

    cout << "File created: " << userinput << endl;
}

void mkdir(vector<string> args)
{
    string userinput = args[1];

    bool proper_path = false;
    string path = SanitizePath(userinput, &proper_path);
    njson *obj = SystemTraversal(SubtractSubstring(path, base_anchor_path));
    // validate if dir exist
    if (obj != nullptr)
    {
        cerr << "Directory already exists." << endl;
        return;
    }

    // small tokenizer based on /
    vector<string> token;
    stringstream check1(path);
    string intermediate;
    string dirparent;

    while (getline(check1, intermediate, '/'))
    {
        token.push_back(intermediate);
    }

    for (int i = 0; i < token.size() - 1; i++)
    {
        dirparent += token[i] + "/";
    }

    njson *dirjson = SystemTraversal(SubtractSubstring(dirparent, base_anchor_path));
    bool layer = VerifyLayer(dirjson);
    bool owner = OwnerCheck(dirjson, session_username);

    // validate layer and dir owner
    if (!layer || !owner)
    {
        cout << "The current user does not have  permission to create the directory" << endl;
        return;
    }

    bool success = filesystem::create_directory(path);
    if (!success)
    {
        cerr << "Failed to create the directory." << endl;
        return;
    }
    string dirname = "";
    for (int i = 0; i < token.size(); i++)
    {
        dirname = token[i];
    }

    njson new_dir = {
        {"objName", dirname},
        {"objType", "directory"},
        {"objLevel", (*dirjson)["objLevel"].get<int>() + 1},
        {"objOwner", session_username},
        {"objCollaborators", njson::array()},
        {"objChildren", njson::array()}};

    new_dir["objCollaborators"].push_back("Admin");
    (*dirjson)["objChildren"].push_back(new_dir);
    WriteChangeToSystemJsonFile();
    cout << "Directoy created. " << endl;
}

void exitmodule(bool *running)
{
    *running = false;
}
