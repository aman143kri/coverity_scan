/*	TextProcessing.cpp
*
*	Source code organizational unit for all functions that manipulates strings.\
*
*/

#include "../headers/modules.h"
#include <bits/stdc++.h>

using namespace std;

//ConvertArgv
string ConvertArgv(char * text) {
	string cmdline(text);
	return cmdline;
}

string SubtractSubstring(string full, string sub) {
	string result = "";
	unsigned int len_full = full.length();
	unsigned int len_sub = sub.length(); 
	unsigned int marker = 0;

	if(len_sub<len_full) {
		// iterate through the full string using substring length and check pattern match
		for(int unsigned i=0; i<len_sub; i++) {
			if(sub[i]!=full[i]) {
				break;
			} else {
				marker += 1;
			}
		}
	}

	// if the marker indicates that the substring spans the full string, we can cut it as it is a perfect substr
	if(marker==len_sub) {
		result = full.substr(marker, len_full);
	}

	return result;
}


//Sanitize Path
/*INPUT 
* This will take a string which is the intended path and
* a pointer to a boolean
* 
* the intended path may be 
* an absolute path-- starts from the user's base directory 
* (filesys for admin and root for  otheruser)
* or a relative path -- starts from the current directory user is in
*
* valid indicates if the sanization was successful
* namely if it found that the user attempted to escape to a directory they shouldn't be able to
* if escape was attempted then *valid will be set to false
* otherwise it will be true
*/
/* OUTPUT
* on success the method will return a 
* true path-- a path relative to operating system, not restricted to our system
* the output will always begin with /
* and end with the name of the last file or directory
* (in otherwords, it will never end with / even in the case of a directory)
*/

string SanitizePath(string pathInput, bool* valid) {
	//should end up being the user's intended destination
	string intendedDest;

	//setup to split input
	stringstream inputStream(pathInput);
	string temp;
	vector<string> vPath;
	//split it up
	while (getline(inputStream, temp, '/')) {
		vPath.push_back(temp);
	}


	/*if their intended destination does not start with "/"
	* they are giving a relative path
	* start intendedDest at the current location and
	* move the pointer over
	*
	* otherwise their destination will be an absolute path
	* (not to be confused with a true path, which the system uses)
	* 
	*/
	if (pathInput.at(0) != '/') {
		intendedDest = dir_ptr;
	} else {
		intendedDest = base_path;
	}

	//it's easier (I think) to deal with the output as a vector temporarily

	//setup to split intendedDest
	stringstream intendedStream(intendedDest);
	vector<string> vDest;
	//split it up
	while (getline(intendedStream, temp, '/')) {
		vDest.push_back(temp);
	}



	//now go through the rest of the input
	for (vector<string>::iterator iter = vPath.begin(); iter < vPath.end(); ++iter) {
		if (*iter == ".") {
			//don't do anything
		}
		else if (*iter == "..") {
            //okay so we're just gonna check at the end
            //for now as long as they're not tryna escape the entire place then
            //we'll leave it alone
			if (!vDest.empty()) {
					vDest.pop_back();
				}
				else {
					//yell at them! they're being bad!

					//OUTPUT ERROR
					intendedDest.clear();
					*valid = false;
					return intendedDest;

			}
		}
		else {
			//just add to the end
			vDest.push_back(*iter);
		}
	}

	//turn the intended output back into a string
	intendedDest.clear();
	vector<string>::iterator iter = vDest.begin();
	++iter; 

	for (iter = iter;  iter < vDest.end(); ++iter) {
		if(!(*iter).empty()){
			intendedDest.append("/");
			intendedDest.append(*iter);
		}
		
	}
	
    /* check that their intended destination actually starts at their relative base path!
	* (particularly important in case they gave an absolute path)
	*/
	*valid = true;
	//due to the potential of intendedDest being base_path minus the "/" at the end
	//we will be adding an additional check
	string intendedDestX = intendedDest + "/";
	if (intendedDest.find(base_path) != 0 && intendedDestX.find(base_path) != 0 ) {
		intendedDest.clear();
		*valid = false;
	} 
	return intendedDest;
}