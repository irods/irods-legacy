flickrOAuthExample1 {
	msiAddKeyVal(*oauth_params, "consumer_key", "$CONSUMER_KEY");
 	msiAddKeyVal(*oauth_params, "consumer_secret", "$CONSUMER_SECRET");
	msiFlickrOAuthExample1(*oauth_params, *KV_out)
	writeKeyValPairs("stdout", *KV_out, ": ");
}
INPUT null
OUTPUT ruleExecOut