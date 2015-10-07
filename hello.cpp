#include <iostream>
#include "include/appdynamics_sdk.h"
#include <thread>
#include <chrono>
#include <stdio.h>
#include <string.h>

using namespace std;

class Payload {
public:
  Payload(string method) : method(method) {}
  static bool bt_payload_reflector(APPD_SDK_BT_PAYLOAD_COMPONENT_TYPE payloadComponentType APPD_SDK_PARAM_IN,
                                   const char* payloadComponentName APPD_SDK_PARAM_IN,
                                   void* payloadVoid APPD_SDK_PARAM_IN,
                                   char* buffer APPD_SDK_PARAM_OUT,
                                   unsigned buffer_size APPD_SDK_PARAM_IN) {
    Payload* payload = (Payload*)payloadVoid;
    switch (payloadComponentType) {
    case APPD_BT_PAYLOAD_COMPONENT_TYPE(bt_name):
      {
        memcpy(buffer, payload->method.c_str(), payload->method.length() + 1);
        return true;
      }
    default:
      {
        return false;
      }
    }
  }
private:
  string method;
};

static bool backend_payload_reflector(
	APPD_SDK_BACKEND_COMPONENT_TYPE payloadComponentType    APPD_SDK_PARAM_IN,
	void* payload                                           APPD_SDK_PARAM_IN,
	char* displayName                                       APPD_SDK_PARAM_OUT,
	unsigned sizeDisplayName                                APPD_SDK_PARAM_IN,
	char* identifyingPropertyName                           APPD_SDK_PARAM_OUT,
	unsigned sizeIdentifyingPropertyName                    APPD_SDK_PARAM_IN,
	char* identifyingPropertyValue                          APPD_SDK_PARAM_OUT,
	unsigned sizeIdentifyingPropertyValue                   APPD_SDK_PARAM_IN,
	bool *resolveBackends)
{
	
	cout << payloadComponentType << "\n"; 
	switch (payloadComponentType)
	{
	case APPD_BACKEND_COMPONENT_TYPE(display_name):
	    {
            char* name = (char*)payload;
            size_t sizetBufferSize = (size_t)sizeDisplayName;
            strncpy(displayName, name, strlen(name) + 1);
            return true;
	    }
        break;

	case APPD_BACKEND_COMPONENT_TYPE(resolveBackends):
	{
        *resolveBackends = false;
		return true;
	}

	default:
		break;
	}
	return false;
}


int main()
{
    	std::cout << "Hello, world!\n";

	APPD_SDK_ENV_RECORD env[8];
    
env[0][0] = APPD_SDK_ENV_CONTROLLER_HOST;
env[0][1] = "";
 
env[1][0] = APPD_SDK_ENV_CONTROLLER_PORT;
env[1][1] = "8090";
 
env[2][0] = APPD_SDK_ENV_CONTROLLER_SSL;
env[2][1] = "0";
 
env[6][0] = APPD_SDK_ENV_ACCOUNT_NAME;
env[6][1] = "customer1";
env[7][0] = APPD_SDK_ENV_ACCOUNT_ACCESS_KEY;
env[7][1] = ""; 
     
env[3][0] = APPD_SDK_ENV_APPLICATION;
env[3][1] = "C++SDK";
 
env[4][0] = APPD_SDK_ENV_TIER;
env[4][1] = "SDK";
 
env[5][0] = APPD_SDK_ENV_NODE;
env[5][1] = "SDK-0";

APPD_SDK_STATUS_CODE res = APPD_SUCCESS;
//res = appdynamics_sdk_init(env, sizeof(env)/sizeof(env[0]));
res=appdynamics_sdk_init(env,8);

int x, SnapshotCount;
SnapshotCount = 0;

std::this_thread::sleep_for (std::chrono::seconds(6));
cout << "Starting\n";

 APPD_SDK_HANDLE_BT btHandle = NULL, backendHandle; 

for (x=1; x<20; x++){

	btHandle = NULL;
	Payload payload("BT_Name");
	cout << "Before BT begin\n";
	res = appdynamics_bt_begin(  APPD_BT_TYPE(native),
                                     Payload::bt_payload_reflector,
                                     &payload,
                                     &btHandle);
 	//cout << "BT Handle: " << btHandle << "\n";
	cout << "Before if------\n";
	if (appdynamics_bt_isSnapshotEnabled(btHandle))
		{
		 SnapshotCount++;
		 cout << "Add User Data\n";
		 const char nameChar[] = "name";
		 const char valueChar[] = "exec pm..load_bogie_risk_wl 'lbrp', 10135, 'U'"; 
		 //char valueChar[2];
		 //sprintf(valueChar, "%d", SnapshotCount);
		 //const char valueChar[] = "value"; 
        	 APPD_SDK_STRING* name = APPD_SDK_STRING_new();
        	 APPD_SDK_STRING* value = APPD_SDK_STRING_new();

		 APPD_SDK_STRING_assign(name, nameChar, sizeof(nameChar)-1);
		 APPD_SDK_STRING_assign(value, valueChar, sizeof(valueChar)-1);
		 cout << "ADDUSERDATA# " << SnapshotCount << "\n";
		 appdynamics_bt_addUserData(btHandle,name,value);
        	 APPD_SDK_STRING_delete(name);
        	 APPD_SDK_STRING_delete(value);
		}

	char backendName[] = "FakeBackend";
	char backendDetailsChar[] = "BackendDetails";
	APPD_SDK_STRING* backendDetails = APPD_SDK_STRING_new();
	APPD_SDK_STRING_assign(backendDetails, backendDetailsChar, sizeof(backendDetailsChar)-1);

	appdynamics_backend_begin(
			btHandle,
			APPD_BACKEND_TYPE(db),
			backend_payload_reflector,
			(void*)backendName,
			NULL,
			NULL,
			&backendHandle);
	appdynamics_backend_setDetails(backendHandle,
					backendDetails);
	APPD_SDK_STRING_delete(backendDetails);

	std::this_thread::sleep_for (std::chrono::seconds(2));
	res = appdynamics_backend_end_success(
			backendHandle);


	std::this_thread::sleep_for (std::chrono::seconds(6));
	cout << "Loops: " << x << "\n";
	cout << "Snapshots: " << SnapshotCount << "\n";
	cout << "Done\n\n";

	res = appdynamics_bt_end_success(btHandle);

	}


appdynamics_sdk_term();
//getchar();

}
