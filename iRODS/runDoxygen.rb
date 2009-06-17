#!/usr/bin/env ruby
=begin
 Script to populate the Doxygen config file and run doxygen on the iRODS
 source code
 16/April/09
 Adil Hasan, University of Liverpool

 You may need to install: imagemagick, texlive, and
 texlive-latex-extra, in addition to Doxygen.  
 If you re-run this script, you may need to remove the directories
 html, rtf and latex under doc to avoid errors ("Could not extract
 bounding box from .eps!").  
 This script will typically take a few hours to complete.
=end

# Check that doxygen exists on the machine if not print out a nice comment and stop
def doxygen_exists()
	# Check that doxygen exists
	#
	doxygen_exe = "doxygen"
	doxy_exists = false
	path_array = ENV['PATH'].split(':')
	for indx in 0...path_array.length do
		doxygen_path = File.join(path_array[indx], doxygen_exe)
		if (File.exist?(doxygen_path)) then
			doxy_exists = true
		end
	end
	return doxy_exists
end

def read_config(cfg_file, doxy_keys)
	# Read in the config file
	#
		
	doxy_cfg = Hash.new()
	if (!File.exist?(cfg_file)) then
		puts("Error: Cannot open doxygen.config file!")
		exit(1)
	end
	file = File.new(cfg_file, "r")
	while (line=file.gets())
		if ((line =~ /#/) == nil) then
			tkey, tvalue = line.split("=")
			key = tkey.strip()
			if (tvalue): value = tvalue.strip() end
			if (doxy_keys.include?(key)): doxy_cfg[key] = value end
		end
	end
	file.close()
	return doxy_cfg
end


# Finally run the doxygen command with the config file

def main()
	# Main function
	#
	doxy_keys = ["PROJECT_NAME", "PROJECT_NUMBER", "OUTPUT_DIRECTORY",
	"STRIP_FROM_PATH", "INPUT"]
	input_file = "./config/doxygen.config"
	output_file = "./config/doxygen-irods.cfg"

	if (!doxygen_exists())
		puts("Error: doxygen executable not found in your path or not installed\n\n")
		exit(1)
	end
	
	doxy_cfg = read_config(input_file, doxy_keys)
	# Prompt for new values for config parameters
	puts("This script generates html, latex and rtf documentation for iRODS")
	puts("The output documentation is under the OUTPUT_DIRECTORY.\n\n")
	puts("Enter config parameter values. Default values in brackets")
	puts("Output configs will be written to doxygen-irods.cfg\n\n")
	for key in doxy_cfg.keys
		# Skip the strip_from_path it is the same as the input
		if ((key =~ /STRIP_FROM_PATH/) != nil): next end
		if (doxy_cfg[key] != nil) then
			puts("Enter #{key} (#{doxy_cfg[key]}):")
			tvalue = gets()
			value = tvalue.strip()
			if (value.length() > 0): doxy_cfg[key] = value end
		else
			puts("Enter #{key}():")
			tvalue = gets()
			value = tvalue.strip()
			if (value.length() > 0): doxy_cfg[key] = value end
		end
	end
	
	doxy_cfg["STRIP_FROM_PATH"] = doxy_cfg["OUTPUT_DIRECTORY"]

	# Read in the config file and write out the contents with new values 
	# to a new output file.
	in_file = File.new(input_file, "r")
	if (File.exist?(output_file)) then
		puts("Warning doxygen-irods.cfg file already exists. Overwrite y/n [n]:")
		response = gets()
		if (response.strip() =~ /n/) then
			exit()
		end
	end
	out_file = File.new(output_file, "w")
	while (line = in_file.gets()) do
		if (line =~ /=/) then
			tkey, tvalue = line.split("=")
			key = tkey.strip()
			if (doxy_keys.include?(key)) then
				line = [tkey, doxy_cfg[key]].join(" = ")
			end
		end
		out_file.puts(line)
	end
	in_file.close()
	out_file.close()
	
	# Now run the doxygen command
	puts("Running the doxygen command to generate documentation.")
	puts("This will take some time. Go get some tea...")
	system("doxygen #{output_file}")
	if ($? != 0) then
		puts("Error: Failed generating the documentation. Return Code: #{$?}")
	else
		puts("Finished generating documentation!")
	end
end

main()
