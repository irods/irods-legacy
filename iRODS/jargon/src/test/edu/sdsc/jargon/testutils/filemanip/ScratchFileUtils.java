/**
 *
 */
package edu.sdsc.jargon.testutils.filemanip;

import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.*;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;

import java.util.Properties;
import java.util.zip.CRC32;
import java.util.zip.CheckedInputStream;
import java.util.zip.Checksum;

import edu.sdsc.jargon.testutils.TestingPropertiesHelper;
import edu.sdsc.jargon.testutils.TestingUtilsException;

/**
 * @author Mike Conway, DICE (www.irods.org)
 * @since 11/03/2009 common utilities to manipulate and validate scratch files
 *        for unit testing
 */

// FIXME: validation of trailing '/' on path in props?

public class ScratchFileUtils {
	private Properties testingProperties = new Properties();
	private TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();

	public ScratchFileUtils(Properties testingProperties) {
		this.testingProperties = testingProperties;
	}
	
	public ScratchFileUtils() throws TestingUtilsException {
		// go ahead and try and derive properties from a known properties file location
		testingProperties = testingPropertiesHelper.getTestProperties();
	}

	/**
	 * Create the scratch dir as described in testing.properties if it does not
	 * already exist. * @param pathUnderScratch <code>String</code> giving the
	 * relative path of the file/directory underneath the scratch area (no
	 * leading / delim is necessary
	 */
	public void createScratchDirIfNotExists(String pathUnderScratch) {
		File scratchDir = new File(testingProperties
				.getProperty(GENERATED_FILE_DIRECTORY_KEY)
				+ pathUnderScratch);
		scratchDir.mkdirs();
	}

	public void createBaseScratchDir() {
		createScratchDirIfNotExists("");
	}

	/**
	 * Check if the given file exists in the scratch area
	 * 
	 * @param pathUnderScratch
	 *            <code>String</code> giving the relative path of the
	 *            file/directory underneath the scratch area (no leading / delim
	 *            is necessary
	 * @return
	 */
	public boolean checkIfFileExistsInScratch(String pathUnderScratch) {
		File targetFile = new File(testingProperties
				.getProperty(GENERATED_FILE_DIRECTORY_KEY)
				+ pathUnderScratch);

		return targetFile.exists();
	}

	public void createDirectoryUnderScratch(String relativePath) {

		createScratchDirIfNotExists(relativePath);

	}

	/**
	 * Convenience method to tack the relative path and file name to the known
	 * scratch path, while creating any necessary intermediate directories
	 * 
	 * @param filePathAndOrName
	 *            <code>String</code> giving any necessary path info as well as
	 *            file name. No leading '/' used
	 * @return <code>String</code> absolute path to the file name, up to the
	 *         last subdirectory, with a trailing '/'
	 */
	public String createAndReturnAbsoluteScratchPath(String path) {

		// this creates intermediate directories
		createScratchDirIfNotExists(path);

		StringBuilder pathBuilder = new StringBuilder();
		pathBuilder.append(testingProperties
				.getProperty(GENERATED_FILE_DIRECTORY_KEY));
		pathBuilder.append(path);
		pathBuilder.append('/');
		return pathBuilder.toString();
	}

	/**
	 * @param pathUnderScratch <code>String</code> with relative file path under scratch (no leading '/')
	 * @return <code>long</code> with the file's checksum value
	 * @throws TestingUtilsException
	 */
	public long computeFileCheckSum(String pathUnderScratch) throws TestingUtilsException {

		StringBuilder pathBuilder = new StringBuilder();
		pathBuilder.append(testingProperties
				.getProperty(GENERATED_FILE_DIRECTORY_KEY));
		pathBuilder.append(pathUnderScratch);
		
		
		long checksum = 0;
		
		try {
			FileInputStream file = new FileInputStream(pathBuilder.toString());
			CheckedInputStream check = new CheckedInputStream(file, new CRC32());
			BufferedInputStream in = new BufferedInputStream(check);
			while (in.read() != -1) {
				// Read file in completely
			}
			checksum = check.getChecksum().getValue();
		} catch (FileNotFoundException e) {
			StringBuilder builder = new StringBuilder();
			builder.append("file not found when computing checksum:");
			builder.append(pathBuilder.toString());
			throw new TestingUtilsException(builder.toString(), e);
		} catch (IOException e) {
			StringBuilder builder = new StringBuilder();
			builder.append("ioexception when computing checksum on file:");
			builder.append(pathBuilder.toString());
			throw new TestingUtilsException(builder.toString(), e);
		}
		
		return checksum;
	}

}
