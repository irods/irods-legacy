flickrOAuthExample2 {
	msiAddKeyVal(*oauth_params, "consumer_key", "$CONSUMER_KEY");
	msiAddKeyVal(*oauth_params, "consumer_secret", "$CONSUMER_SECRET");
	msiAddKeyVal(*oauth_params, "token", "$REQUEST_TOKEN");
 	msiAddKeyVal(*oauth_params, "token_secret", "$REQUEST_TOKEN_SECRET");
 	msiAddKeyVal(*oauth_params, "verifier", "$VERIFIER");
	msiFlickrOAuthExample2(*oauth_params, *KV_out)
	writeKeyValPairs("stdout", *KV_out, ": ");
}
INPUT null
OUTPUT ruleExecOut