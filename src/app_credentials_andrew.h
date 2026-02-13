/*
 * Used to define secret credentials used by the program.
 *
 * User should duplicate this file into 'app_credentials.h' and fill out the
 * fields prior to building.
 * Warning: the resulting 'app_credentials.h' should not be tracked in version
 * control. This has already been setup in .gitignore.
 */

#ifndef AUTOLAB_APP_CREDENTIALS_H_
#define AUTOLAB_APP_CREDENTIALS_H_

/* The domain of the autolab service. Do NOT include the trailing slash.
 * Must use 'https' instead of 'http' in production.
 */
const std::string server_domain = "https://autolab.andrew.cmu.edu";

/* The client_id and client_secret generated when registering the app */
const std::string client_id = "1a0c4d22f9bd326476b8fd058f8d0a63d619d7485d22064a2e6a734f7bbfbcea";
const std::string client_secret = "f1559f2736a5e2469e783874cf4ef2faf87e3233aaac0d07a68235279b31741c";

/* The redirect uri used when registering the app.
 * For clients that use device_flow, this should have been
 *   "<host>/device_flow_auth_cb"
 */
const std::string redirect_uri = "https://autolab.andrew.cmu.edu/device_flow_auth_cb";

/* Key and iv used to encrypt tokens.
 * Note: lengths of strings must be correct:
 *  - key: 256 bits, or 32 regular chars
 *  - iv : 128 bits, or 16 regular chars
 * For safety reasons, please keep these declarations as macros instead of
 * variables.
 */
#define crypto_key ((unsigned char *)"EnterYourVerySecretCryptoKeyHere")
#define crypto_iv  ((unsigned char *)"YourVerySecretIV")

#endif /* AUTOLAB_APP_CREDENTIALS_H_ */