/*	UserManagement.cpp
*
*	Source code organizational unit for functions and classes related to user management,
*    like AdminCreateUser(), AddCollaborator(), and other user-related tasks.
*
*/


#include "../headers/modules.h"


//helper for AdminCreateUser, append user to keymapper.json
void AppendToKeymapper(const njson& user_keyfile_obj) {
    // Construct the file path
    filesys::path path_keymapper(base_anchor_path + "filesys/keymap/keymapper.json");

    // Read the existing JSON data
    njson keymapper;
    std::ifstream infile(path_keymapper);
    infile >> keymapper;
    if (infile.fail()) {
        std::cerr << "Error parsing JSON." << std::endl;
        return;
    }
    infile.close();

    // Add the new pair
    keymapper[user_keyfile_obj["username"].get<std::string>()] = user_keyfile_obj["secretkey"];


    // Write the modified JSON object back to the file
    std::ofstream outfile(path_keymapper);
    // Attempt to write to the file
    outfile << std::setw(4) << keymapper;
    if (outfile.fail()) {
        std::cerr << "Error writing JSON." << std::endl;
        return;
    }
    outfile.close();
    //std::cout << "appended user to keymapper.json.\n";
}


//helper for AdminCreateUser, update the share.json
void UpdateSharedJson(const std::string& username) {
    filesys::path path_shared_tracker(base_anchor_path + "filesys/metadata/shared.json");
    std::ifstream sharedFile(path_shared_tracker);
    njson shared_tracker = njson::parse(sharedFile);

    shared_tracker[username] = njson::array();

    std::ofstream sharedFileOut(path_shared_tracker);
    sharedFileOut << shared_tracker.dump(4);
    sharedFileOut.close();
    //std::cout << "share.json entry created successfully." << std::endl;

}

//Write the json tree to file
void WriteChangeToSystemJsonFile(){
    std::ofstream ofs(base_anchor_path+"filesys/metadata/system.json");
        if (!ofs) {
            std::cerr << "Failed to open file for writing\n";
            return;
        }
    ofs << (*system_tree).dump(4);
    ofs.close();
    //std::cout << "The system.json file has been updated successfully.\n";
}

//helper for AdminCreateUser, update the system.json
void UpdateSystemJson(const std::string& username) {
    filesys::path path_root("filesys/origin/root/");
    njson* root_path_ptr = SystemTraversal(path_root);
    // Check if the traversal was successful and the pointer is not null
    if (root_path_ptr == nullptr) {
        std::cerr << "Root path not found in system JSON." << std::endl;
        return;
    }
    njson storage_user = {
        {"objName", "storage-" + username},
        {"objType", "directory"},
        {"objLevel", 1},
        {"objOwner", username},
        {"objCollaborators", njson::array({"Admin", username})},
        {"objChildren", njson::array()}
    };
    njson shared_user = {
        {"objName", "shared-" + username},
        {"objType", "sdirectory"},
        {"objLevel", 1},
        {"objOwner", "System"},
        {"objCollaborators", njson::array({"Admin", username})},
        {"objChildren", njson::array()}
    };
    (*root_path_ptr)["objChildren"].push_back(storage_user);
    (*root_path_ptr)["objChildren"].push_back(shared_user);
    WriteChangeToSystemJsonFile();
}



//helper for AdminCreateUser and AddCollaborator, update the system.json
//find the sysObject and add the target user to its collaborator field
//call function to write the system tree to file.
bool UpdateSystemJsonAddCollab(const std::string& filePath, const std::string& targetUser) {
    std::string path_for_SystemTraversal= filePath.substr(base_anchor_path.length());
    //std::cout << " path for SystemTraversal: "<< path_for_SystemTraversal << std::endl;

    filesys::path file_path(path_for_SystemTraversal);
    njson* file_path_ptr = SystemTraversal(path_for_SystemTraversal);

    // Check if the traversal was successful and the pointer is not null
    if (file_path_ptr == nullptr) {
        //std::cerr << "File path not found in system JSON." << std::endl;
        return false;
    }

    (*file_path_ptr)["objCollaborators"].push_back(targetUser);

    WriteChangeToSystemJsonFile();

    return true;
}


void AdminCreateUser(const std::string& username) {
    //Create keyfile for the user
    njson user_keyfile_obj = GenerateKeyfile(username);

    AppendToKeymapper(user_keyfile_obj);

    //create user directories under root
    const filesys::path root_path(base_anchor_path+"filesys/origin/root/");
    std::string storage_username ="storage-" + username + "/";
    std::string shared_username ="shared-" + username + "/";
    filesys::path storage_path(storage_username);
    filesys::path shared_path(shared_username);
    filesys::create_directory(root_path/storage_path);
    filesys::create_directory(root_path/shared_path);
    //std::cout << "Created user directories." << endl;

    UpdateSharedJson(username);
    UpdateSystemJson(username);
    UpdateSystemJsonAddCollab(base_anchor_path+"filesys/origin/root/",username);
    std::cout << "User " << username << " created successfully." << std::endl;
}


//helper for AddCollaborator, update the shared.json
bool UpdateSharedJsonAddCollab(const std::string& filePath, const std::string& targetUser) {
   // Find the last path separator as filename
    size_t pos = filePath.find_last_of("/");
    std::string filename = filePath.substr(pos + 1);

    njson user_shrObj = {
    		{"fileName", filename},
    		{"filePath", filePath}
    };

    filesys::path path_shared_tracker(base_anchor_path + "filesys/metadata/shared.json");
    std::ifstream sharedFile(path_shared_tracker);

    if (!sharedFile.is_open()) {
        std::cerr << "Failed to open shared JSON file." << std::endl;
        return false;
    }

    njson shared_tracker;
    sharedFile >> shared_tracker; // Parse the JSON content

    (shared_tracker[targetUser]).push_back(user_shrObj);
    sharedFile.close();

    std::ofstream sharedFileOut(path_shared_tracker);
    if (!sharedFileOut.is_open()) {
        std::cerr << "Failed to open shared JSON file for writing." << std::endl;
        return false;
    }

    sharedFileOut << shared_tracker.dump(4);
    sharedFileOut.close();
    //std::cout << "share.json added file path for this user successfully." << std::endl;
    return true;
}



bool AddCollaborator(const std::string& filePath, const std::string& targetUser) {
    // Call the helper functions and check their return values
    bool systemJsonUpdated = UpdateSystemJsonAddCollab(filePath, targetUser);
    bool sharedJsonUpdated = UpdateSharedJsonAddCollab(filePath, targetUser);

    if (systemJsonUpdated && sharedJsonUpdated) {
        //std::cout << "Collaborator added successfully.\n";
        return true;
    } else {
        //std::cout << "Cannot add collaborator.\n";
        return false;
    }
}

