using namespace std;

#include <bits/stdc++.h>
#include "../headers/modules.h"
#include "../headers/cipher.h"

//global variables defined in modules.h
string base_anchor_path = sys_getcwd();
string base_path = base_anchor_path + "filesys/";
string dir_ptr = "";
string session_username = "";
njson * system_tree = nullptr;

// main function
int main(int argc, char * argv[]) {
	//printf("[main] Program starting with %d arguments.\n", argc);

	// start with checking if the filesystem is active
	bool filesys_exists = CheckFilesys();

	if(not filesys_exists) {
		// generate the initial filesystem.
		InitSetup();
		printf("Filesystem generated.\n");
	} else {
		// check for args from the command line
		string keyfilename = "";

		if(argc>1){
			keyfilename = ConvertArgv(argv[1]);
		}

		//printf("[main] keyfilename obtained: %s\n", keyfilename.c_str());

		bool validated = false;
		// validate the key and get the username.
		session_username = ValidateKey(&validated, keyfilename);		

		if(not validated) {
			printf("Invalid keyfile.\n");
			exit(1);
		} else {
			printf("Logged in as %s\n", session_username.c_str());

			//printf("[main] Finished loading the system tree.\n");
			// load in the metadata filesystem into our system_tree ptr.
			sys_load_system_tree();		

			// set the enforced root to be deeper in for regular users
			if(session_username!="Admin") {
				base_path += "origin/root/";
			}

			// we'll land both admin and user near the filesystem
			dir_ptr = base_anchor_path + "filesys/origin/root/";

			//printf("[main] Running with:\ndir_ptr: %s,\nbase_path: %s\n",dir_ptr.c_str(), base_path.c_str());

			bool running = true;
			// begin main loop			
			while(running) {
				// get user input				
				getAndExecuteCommand(&running);

			}
		}

		// clean-up code
		if(system_tree!=nullptr) {
			delete system_tree;
		}

	}


	return 0;
}
