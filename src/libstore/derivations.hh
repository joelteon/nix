#ifndef __DERIVATIONS_H
#define __DERIVATIONS_H

typedef struct _ATerm * ATerm;

#include "hash.hh"
#include "util.hh"

#include <map>


namespace nix {


/* Extension of derivations in the Nix store. */
const string drvExtension = ".drv";


/* Abstract syntax of derivations. */

struct DerivationOutput
{
    Path path;
    string hashAlgo; 	/* hash used for expected hash computation */
    string hash; 		/* expected hash, may be null */
    DerivationOutput()
    {
    }
    DerivationOutput(Path path, string hashAlgo, string hash)
    {
        this->path = path;
        this->hashAlgo = hashAlgo;
        this->hash = hash;
    }
};

struct DerivationStateOutput
{
    Path statepath;
    string componentHash;
    string hashAlgo;
    string hash;
    string stateIdentifier;				//the identifier
    string enabled;						//enable or disable state
    string shareType;						//none, full, group
    string synchronization;				//none (no locks), exclusive-lock, recursive-exclusive-lock
    
    string commitReferences;			//TODO none, direct, recursive-all
    string commitBinaries;				//TODO list of binaries that need (or not) to be committed when these binaries are called
         
    string createDirsBeforeInstall;		//if true: creates state dirs before installation
	string runtimeStateArgs;			//if not empty: these are the runtime parameters where state can be found (you can use $statepath here)
    
    string username;	
    
    string sharedState;					//Path to share state From		
    
    string externalState;				//a statePath not not in the stateStore (Official hack)
    
    DerivationStateOutput()
    {
    }
    
    //TODO add const ??
    DerivationStateOutput(Path statepath, string componentHash, string hashAlgo, string hash, string stateIdentifier, string enabled, string shareType, string synchronization, string createDirsBeforeInstall, string runtimeStateArgs, string username, string sharedState, string externalState, bool check=true)
    {
        if(check){
	        if(shareType != "none" && shareType != "full" && shareType != "group")
	        	throw Error(format("shareType '%1%' is not a correct type") % shareType);
	        if(synchronization != "none" && synchronization != "exclusive-lock" && synchronization != "recursive-exclusive-lock")
	        	throw Error(format("synchronization '%1%' is not a correct type") % synchronization);
	        if(username == "")
	        	throw Error(format("Username cannot be empty"));
	        if(stateIdentifier == "__EMTPY__" || stateIdentifier == "__NOSTATE__")
	        	throw Error(format("the stateIdenfier cannot be this value '%1%'") % stateIdentifier);
	        if(runtimeStateArgs == "__NOARGS__")
	        	throw Error(format("the runtimeStateArgs cannot be this value '%1%'") % runtimeStateArgs);
			if(externalState != "" && sharedState != "")
				throw Error(format("You cannot have an externalState and sharedState at the same time"));	        
        }
        
        //TODO
        //commitReferences
        //commitBinaries
        
        this->statepath = statepath;
        this->componentHash = componentHash;
        this->hashAlgo = hashAlgo;
        this->hash = hash;
        this->stateIdentifier = stateIdentifier;
        this->enabled = enabled;
        this->shareType = shareType;
        this->synchronization = synchronization;
        this->createDirsBeforeInstall = createDirsBeforeInstall;
        this->runtimeStateArgs = runtimeStateArgs;
        this->username = username;
        this->sharedState = sharedState;
		this->externalState = externalState;        
    }
    
    bool getEnabled(){
    	return string2bool(enabled);
    }
    
    bool getCreateDirsBeforeInstall(){
    	return string2bool(createDirsBeforeInstall);
    }
    
    /*
     * This sets all the paramters to "" to ensure they're not taken into account for the hash calculation in primops.cc 
     */
    void clearAllRuntimeParamters(){
    	this->statepath = "";
    	this->componentHash = "";
        //this->hashAlgo;						//Clear this one?
        //this->hash;							//Clear this one?
        //this->stateIdentifier;				//Changes the statepath directly
        this->enabled = "";
        this->shareType = "";
        this->synchronization = "";
        this->createDirsBeforeInstall = "";
        this->runtimeStateArgs = "";
        //this->username;						//Changes the statepath directly
        this->sharedState = "";
        this->externalState = "";
         
    }
};

struct DerivationStateOutputDir
{
    string path;
    string type;					//none, manual, interval, full
    string interval;				//type int
    DerivationStateOutputDir()
    {
    }
    DerivationStateOutputDir(string path, string type, string interval)
    {
        if(type != "none" && type != "manual" && type != "interval" && type != "full")
        	throw Error(format("interval '%1%' is not a correct type") % type);
        
        this->path = path;
        this->type = type;
        this->interval = interval;
    }
    
    int getInterval(){
    	if(interval == "") 
          return 0;
    	else{
	    	int i;
	    	if (!string2Int(interval, i)) throw Error("interval is not a number");
	    	return i;
    	}
    }
    
    //sort function
	bool operator<(const DerivationStateOutputDir& a) const { return path < a.path; }      
};


typedef std::map<string, DerivationOutput> DerivationOutputs;
typedef std::map<string, DerivationStateOutput> DerivationStateOutputs;
typedef std::map<string, DerivationStateOutputDir> DerivationStateOutputDirs;


/* For inputs that are sub-derivations, we specify exactly which
   output IDs we are interested in. */
typedef std::map<Path, StringSet> DerivationInputs;
typedef std::map<string, string> StringPairs;

struct Derivation
{
    DerivationOutputs outputs; /* keyed on symbolic IDs */
    DerivationStateOutputs stateOutputs; /* TODO */
    DerivationStateOutputDirs stateOutputDirs; /* TODO */
    DerivationInputs inputDrvs; /* inputs that are sub-derivations */
    PathSet inputSrcs; /* inputs that are sources */
    string platform;
    Path builder;
    Strings args;
    StringPairs env;
};


/* Hash an aterm. */
Hash hashTerm(ATerm t);

/* Write a derivation to the Nix store, and return its path. */
Path writeDerivation(const Derivation & drv, const string & name);

/* Parse a derivation. */
Derivation parseDerivation(ATerm t);

/* Parse a derivation. */
ATerm unparseDerivation(const Derivation & drv);

/* Check whether a file name ends with the extensions for
   derivations. */
bool isDerivation(const string & fileName);

 
}


#endif /* !__DERIVATIONS_H */
