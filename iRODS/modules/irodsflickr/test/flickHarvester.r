flickHarvester {
	msiAddKeyVal(*oauth_params, "target_collection", "/example/home/public/Photos");
	msiAddKeyVal(*oauth_params, "consumer_key", "$CONSUMER_KEY");
	msiAddKeyVal(*oauth_params, "consumer_secret", "$CONSUMER_SECRET");
	msiAddKeyVal(*oauth_params, "token", "$ACCESS_TOKEN");
 	msiAddKeyVal(*oauth_params, "token_secret", "$ACCESS_TOKEN_SECRET");
	msiFlickHarvester(*oauth_params, *KV_out)
	writeKeyValPairs("stdout", *KV_out, ": ");
}
INPUT null
OUTPUT ruleExecOut