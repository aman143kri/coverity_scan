/*	Filesys.cpp
*
*	Source code organizational unit for all functions that deals with the filesys structure.
*
*/

#include <fstream>
#include <vector>
#include "../headers/modules.h"

/*#include <cryptopp/aes.h>
#include <cryptopp/modes.h>
#include <cryptopp/filters.h>
#include <cryptopp/cryptlib.h>
#include <cryptopp/osrng.h>*/

using namespace std;
const int MIN_INT_VALUE = -2147483648;

// sys_getcwd - gets the anchor point relative to our executable
string sys_getcwd() {
	filesys::path anchor = filesys::current_path();
	string curwd = anchor.string() + "/";
	return curwd;
}

// sys_load_keymapper - load the keymapper file
njson sys_load_keymapper() {
	// load the keymapoper file from the known location.
	ifstream jfile(filesys::path(base_anchor_path+"filesys/keymap/keymapper.json"));
	njson keymapper;
	if(jfile.is_open()) {
		jfile >> keymapper;
	}
	//printf("[sys_load_keymapper] keymapper file loaded.\n");

	return keymapper;

}

// sys_load_system_tree - load the json
void sys_load_system_tree() {
	// load the system.json file from the known location
	ifstream jfile(filesys::path(base_anchor_path+"filesys/metadata/system.json"));
	if(jfile.is_open()) {
		// create a new json object in heap memory
		system_tree = new njson();
		// load file contents
		jfile >> *system_tree;
		//printf("[sys_load_system_tree] json read into memory %s\n... truncated for brevity.\n", system_tree->dump(4).substr(0, 350).c_str());
	}

	// close the file for best practices
	jfile.close();
}

// CheckFilesys - checks whether the entire filesys structure exists
bool CheckFilesys() {
	// get the base position pointing to the directory of executable on host
	filesys::path position(base_anchor_path);

	// initialize a vector for all the layered paths
	vector<filesys::path> path_blueprints = {
		"keyfiles/",
		"filesys/",
		"filesys/metadata/",
		"filesys/keymap/",
		"filesys/origin/",
		"filesys/origin/root/",
		"filesys/origin/root/storage-Admin",
		"filesys/origin/root/shared-Admin"
	};

	bool filesys_valid = true;
	for(const filesys::path path_blueprint:path_blueprints) {
		filesys::path target = position/path_blueprint;
		//printf("[CheckFilesys] Checking path %s\n", (target).string().c_str());
		if(not (filesys::exists(target) and filesys::is_directory(target))) {
			//printf("[CheckFilesys] Folder structure does not exist!\n");
			filesys_valid = false;
			break;
		}
	}

	return filesys_valid;

}

// HelperWriteJson - takes in path and json object and write to file.
void HelperWriteJson(filesys::path target, njson jfile) {
	// convert json into string an d write it to outfile.
	string contents = jfile.dump(4);
	ofstream outfile(target);

	if(outfile.is_open()) {
		// convert json into string an d write it to outfile.
		string contents = jfile.dump(4);
		ofstream outfile(target);

		//printf("[HelperWriteJson] Writing to outfile %s\n", target.string().c_str());
		//printf("[HelperWriteJson] Adding contents %s\n", contents.c_str());
		outfile << contents;
		outfile.close();
	} else {
		printf("Error while writing to json file.");
	}

}

// HelperBuildInitSystem - builds the default filesystem representation
njson HelperBuildInitSystem() {

	// create the filesysroot
	njson filesysroot = {
		{"objName", "filesys"},
		{"objType", "directory"},
		{"objLevel", -2},
		{"objOwner", "System"},
		{"objCollaborators", njson::array({"Admin"})},
		{"objChildren", njson::array()}
	};

	// create the first layer 
	njson metadata = {
		{"objName", "metadata"},
		{"objType", "directory"},
		{"objLevel", -1},
		{"objOwner", "System"},
		{"objCollaborators", njson::array({"Admin"})},
		{"objChildren", njson::array()}
	};

	njson keymap = {
		{"objName", "keymap"},
		{"objType", "directory"},
		{"objLevel", -1},
		{"objOwner", "System"},
		{"objCollaborators", njson::array({"Admin"})},
		{"objChildren", njson::array()}
	};

	njson origin = {
		{"objName", "origin"},
		{"objType", "directory"},
		{"objLevel", -1},
		{"objOwner", "System"},
		{"objCollaborators", njson::array({"Admin"})},
		{"objChildren", njson::array()}
	};

	// create the second layer
	njson root = {
		{"objName", "root"},
		{"objType", "directory"},
		{"objLevel", 0},
		{"objOwner", "System"},
		{"objCollaborators", njson::array({"Admin"})},
		{"objChildren", njson::array()}
	};

	// create the third layer
	njson storage_admin = {
		{"objName", "storage-Admin"},
		{"objType", "directory"},
		{"objLevel", 1},
		{"objOwner", "Admin"},
		{"objCollaborators", njson::array()},
		{"objChildren", njson::array()}
	};

	njson shared_admin = {
		{"objName", "shared-Admin"},
		{"objType", "sdirectory"},
		{"objLevel", 1},
		{"objOwner", "System"},
		{"objCollaborators", njson::array({"Admin"})},
		{"objChildren", njson::array()}
	};

	// create the special sysfiles
	njson system_json = {
		{"objName", "system.json"},
		{"objType", "file"},
		{"objLevel", 0},
		{"objOwner", "System"},
		{"objCollaborators", njson::array()},
		{"objChildren", njson::array()}
	};

	njson keymapper_json = {
		{"objName", "keymapper.json"},
		{"objType", "file"},
		{"objLevel", 0},
		{"objOwner", "System"},
		{"objCollaborators", njson::array()},
		{"objChildren", njson::array()}
	};

	njson shared_json = {
		{"objName", "shared.json"},
		{"objType", "file"},
		{"objLevel", 0},
		{"objOwner", "System"},
		{"objCollaborators", njson::array()},
		{"objChildren", njson::array()}
	};

	// start fitting the tree together
	root["objChildren"].push_back(storage_admin);
	root["objChildren"].push_back(shared_admin);

	origin["objChildren"].push_back(root);
	keymap["objChildren"].push_back(keymapper_json);
	metadata["objChildren"].push_back(system_json);
	metadata["objChildren"].push_back(shared_json);

	filesysroot["objChildren"].push_back(origin);
	filesysroot["objChildren"].push_back(keymap);
	filesysroot["objChildren"].push_back(metadata);

	return filesysroot;

}


// Initsetup
void InitSetup() {
	//printf("[InitSetup] Generate directories and basic files.\n");

	// start building the directory structure
	filesys::path position(base_anchor_path);
	
	// initialize a vector for all the layered paths
	vector<filesys::path> path_blueprints = {
		"keyfiles/",
		"filesys/",
		"filesys/metadata/",
		"filesys/keymap/",
		"filesys/origin/",
		"filesys/origin/root/",
		"filesys/origin/root/storage-Admin",
		"filesys/origin/root/shared-Admin"
	};

	// build the directory structure from vector
	for(const filesys::path path_blueprint:path_blueprints) {
		try {
			//printf("[Initsetup] Attempting to create directory %s\n", (position/path_blueprint).string().c_str());
			filesys::create_directory(position/path_blueprint);

		} catch(const exception err) {
			printf("Ran into a problem when trying to build the filesystem.\n");
		}
	}
	
	// build files 
	njson admin_keyfile = GenerateKeyfile("Admin");
	
	// create the keymapper file
	filesys::path path_keymapper(base_anchor_path + "filesys/keymap/keymapper.json");
	njson keymapper = {
		{admin_keyfile["username"], admin_keyfile["secretkey"]}
	};

	HelperWriteJson(path_keymapper, keymapper);

	// create the Shared tracker file
	filesys::path path_shared_tracker(base_anchor_path + "filesys/metadata/shared.json");
	njson shared_tracker = {
		{"Admin", njson::array()}
	};

	HelperWriteJson(path_shared_tracker, shared_tracker);

	// create the system file
	filesys::path path_system(base_anchor_path + "filesys/metadata/system.json");
	njson filesysroot = HelperBuildInitSystem();
	HelperWriteJson(path_system, filesysroot);

}


njson GenerateKeyfile(string newUserID){
	//create key using openssl (and create empty njson)
	njson newKeyfile;
	int keySize = 8;
	unsigned char key[keySize];
	if (!RAND_bytes(key, keySize)) {
		//should this fail for some reason we should let user know and return empty json
		cout << "key generation failed!" << endl;
		return newKeyfile;
	}

	//put the key in a char array to cast to standard string
	//(maybe there's a better way but this was the simplest I found)
	char keyHex[keySize*2+1];
	for(int i=0; i < keySize; i++){
		sprintf(keyHex+2*i, "%02X", key[i]);
	}
	string keyStr(keyHex);

	//initialize njson
	newKeyfile = {
		{"username", newUserID},
		{"secretkey", keyStr}
	};

	//write out location for keyfile and pass it off to HelperWriteJson
	string keyfileLoc = base_anchor_path+"keyfiles/keyfile-"+newUserID+".json";
	filesys::path keyfilePath(keyfileLoc);
	HelperWriteJson(keyfilePath, newKeyfile);

	//return the json
	return newKeyfile;
}

//relies on crypto++ instead of openssl
/*njson GenerateKeyfile2(string newUserID){
	//create key using cryptopp
	int keySize = CryptoPP::AES::DEFAULT_KEYLENGTH;
	CryptoPP::AutoSeededRandomPool prng;
	byte key[keySize];

	//whelp for the prvs cast to string we now need to cast to unsigned char lmao 
	unsigned char keyChar[keySize];
	for(int i=0; i < keySize; i++){
		keyChar[i] = static_cast<unsigned char>(key[i]); 
	}

	//put the key in a char array to cast to standard string
	//(maybe there's a better way but this was the simplest I found)
	char keyHex[keySize*2+1];
	for(int i=0; i < keySize; i++){
		sprintf(keyHex+2*i, "%02X", keyChar[i]);
	}
	string keyStr(keyHex);

	//initialize njson
	njson newKeyfile = {
		{"username", newUserID},
		{"secretkey", keyStr}
	};

	//write out location for keyfile and pass it off to HelperWriteJson
	string keyfileLoc = base_anchor_path+"keyfiles/keyfile-"+newUserID+".json";
	filesys::path keyfilePath(keyfileLoc);
	HelperWriteJson(keyfilePath, newKeyfile);

	//return the json
	return newKeyfile;
}*/

njson ReadKeyfile(string soughtUser){
	//create the empty njson first
	njson soughtKeyfile;

	//where to look for keyfile
	string soughtLoc = base_anchor_path+"keyfiles/keyfile-"+soughtUser+".json";
	filesys::path soughtPath(soughtLoc);

	//if it's not there, then return empty json
	if(!(filesys::exists(soughtPath))){
		return soughtKeyfile;
	}

	//if it is open up that file and put it into our return var
	ifstream readStream(soughtPath);
	readStream >> soughtKeyfile;

	//then just close the file and return
	readStream.close();
	return soughtKeyfile;
}

// ValidateKey - authenticates the user-provided key
std::string ValidateKey(bool * validated, std::string filename) {
	string username = "";
	string user_secretkey = "0";
	string stored_secretkey = "1";
	*validated = false;

	//printf("[ValidateKey] Passed in the keyfile name of %s\n", filename.c_str());

	// retieve the user keyfile
	njson user_keyfile = ReadKeyfile(filename);
	if(not user_keyfile.empty()) {
		try {
			username = user_keyfile["username"];
			user_secretkey = user_keyfile["secretkey"];
			//printf("[ValidateKey] User's given secret key: %s\n", user_secretkey.c_str());

			// retrieve the keymapper file - this file is never empty
			njson sys_keymapper = sys_load_keymapper();
			if(sys_keymapper.contains(username)) {
				stored_secretkey = sys_keymapper[username];
				//printf("[ValidateKey] Stored secret key: %s\n", stored_secretkey.c_str());
				*validated = (stored_secretkey == user_secretkey);

			}
		} catch(const exception err) {
				*validated = false;
		}
	}

	return username;
}

njson * SystemTraversal(std::string targetPath) {
	//cout << targetPath << '\n';

	//check that the targetPath isn't null
	if(targetPath.length() == 0) return nullptr;
	
	//Seperate the path into tokens according to the delimiter /
	std::string tmpPath = targetPath;
	std::string delimiter = "/";
	std::vector<std::string> pathList;
	while(targetPath.length() > 0) {
		std::string splitToken = targetPath.substr(0, targetPath.find(delimiter));
		pathList.push_back(splitToken);
		targetPath.erase(0, splitToken.length() + 1);
	}

	//Iterate through the system_tree and try to find inner elements with matching objName, if doesn't exist, return null pointer
	njson * iterTree = system_tree;
	std::string lastSubPath = "";
	int currentObjLevel = MIN_INT_VALUE;
	for (std::string pathToken: pathList) {
		//cout << "round start: " << pathToken << ": " << currentObjLevel << '\n';
		bool subPathExist = false;

		//cout << '\n' << lastSubPath << ": " << currentObjLevel << ": " << (*iterTree).at("objName") << ": " << (*iterTree).at("objLevel") << "\n";
		if((*iterTree).at("objName") == pathToken && (*iterTree).at("objLevel") != currentObjLevel) {
			(*iterTree).at("objName").get_to(lastSubPath);
			currentObjLevel = (*iterTree).at("objLevel").get_to(currentObjLevel);
			continue;
		}

		int childLen = (*iterTree).at("objChildren").size();
		for (int i=0; i<childLen; i++) {
			//cout << (*iterTree).at("objChildren").at(i).at("objName") << ": " << pathToken << '\n';
			if((*iterTree).at("objChildren").at(i).at("objName") == pathToken) {
				iterTree = &(*iterTree).at("objChildren").at(i);
				currentObjLevel = (*iterTree).at("objLevel").get_to(currentObjLevel);
				subPathExist = true;
				break;
			}
		}
		//cout << "round result: " << pathToken << ": sub path: " << subPathExist << '\n';
		if(!subPathExist) {
			//cout << "triggered\n";
			return nullptr;
		}

	}

	//create json pointer based on the navigated path, and return it
	njson::json_pointer tmpPtr("/" + tmpPath);
	njson * ptr = iterTree;
    return ptr;
}


bool DirectoryCheck(njson * node) {
    string obj_type = (*node)["objType"];
    bool result = (obj_type == "directory" or obj_type == "sdirectory");
    //printf("[DirectoryCheck] type check for directory is %s\n", result ? "true":"false");
    return result;
}

//check that the user is a collaborator of the object they wish to read
bool PermissionCheck(njson * node, string targ_name) {
	return std::any_of(std::begin((*node)["objCollaborators"]), std::end((*node)["objCollaborators"]),
		[&](std::string collaborator){return collaborator == targ_name;});
}

//check that the user is an owner of the object they wish to access
bool OwnerCheck(njson * node, string targ_name) {
	string huh = (*node)["objOwner"];
	//printf("[OwnerCheck] The objOwner is %s\n", huh.c_str());
	return (*node)["objOwner"] == targ_name;
}

bool VerifyLayer(njson * node) {
    
	bool contained = true;

	try {
		int layer = (*node)["objLevel"];
    	//printf("[VerifyLayer] The object layer is %d\n", layer);
    	if(session_username=="Admin") {
    	    contained = (layer >= -2);
    	} else {
    	    contained = (layer >= 0); 
    	}   
    	//printf("[VerifyLayer] For user %s, we determined that contained is %s\n", session_username.c_str(), contained ? "true    ":"false");

	} catch(const exception err) {
		contained = false;
	}

    return contained;
}

