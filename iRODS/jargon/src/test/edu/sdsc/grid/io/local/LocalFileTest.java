/**
 *
 */
package edu.sdsc.grid.io.local;

import edu.sdsc.grid.io.FileFactory;
import edu.sdsc.grid.io.GeneralAccount;
import edu.sdsc.grid.io.GeneralFile;
import edu.sdsc.grid.io.GeneralFileSystem;

import edu.sdsc.jargon.testutils.IRODSTestSetupUtilities;
import edu.sdsc.jargon.testutils.TestingPropertiesHelper;
import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.GENERATED_FILE_DIRECTORY_KEY;
import edu.sdsc.jargon.testutils.filemanip.FileGenerator;
import edu.sdsc.jargon.testutils.filemanip.ScratchFileUtils;
import edu.sdsc.jargon.testutils.icommandinvoke.IcommandInvoker;
import edu.sdsc.jargon.testutils.icommandinvoke.IrodsInvocationContext;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.IlsCommand;

import junit.framework.TestCase;

import org.junit.After;
import org.junit.AfterClass;
import static org.junit.Assert.*;

import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Ignore;
import org.junit.Test;

import java.util.Properties;


/**
 * @author Mike Conway, DICE (www.irods.org)
 * @since 2.0.6
 */
public class LocalFileTest {
    private static Properties testingProperties = new Properties();
    private static TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();
    private static ScratchFileUtils scratchFileUtils = null;
    public static final String IRODS_TEST_SUBDIR_PATH = "LocalFileTest";
    private static IRODSTestSetupUtilities irodsTestSetupUtilities = null;

    @BeforeClass
    public static void setUpBeforeClass() throws Exception {
        TestingPropertiesHelper testingPropertiesLoader = new TestingPropertiesHelper();
        testingProperties = testingPropertiesLoader.getTestProperties();
        scratchFileUtils = new ScratchFileUtils(testingProperties);
        irodsTestSetupUtilities = new IRODSTestSetupUtilities();
        irodsTestSetupUtilities.initializeIrodsScratchDirectory();
        irodsTestSetupUtilities.initializeDirectoryForTest(IRODS_TEST_SUBDIR_PATH);
    }

    /**
     * @throws java.lang.Exception
     */
    @AfterClass
    public static void tearDownAfterClass() throws Exception {
    }

    /**
     * @throws java.lang.Exception
     */
    @Before
    public void setUp() throws Exception {
    }

    /**
     * @throws java.lang.Exception
     */
    @After
    public void tearDown() throws Exception {
    }

    /**
     * Test method for
     * {@link edu.sdsc.grid.io.local.LocalFile#getPathSeparator()}.
     */
    @Ignore
    public final void testGetPathSeparator() {
        fail("Not yet implemented");
    }

    /**
     * Test method for
     * {@link edu.sdsc.grid.io.local.LocalFile#getPathSeparatorChar()}.
     */
    @Ignore
    public final void testGetPathSeparatorChar() {
        fail("Not yet implemented");
    }

    /**
     * Test method for
     * {@link edu.sdsc.grid.io.local.LocalFile#copyTo(edu.sdsc.grid.io.GeneralFile)}
     * .
     */
    @Test
    public final void testCopyToGeneralFileUsingSmallSourceFile()
        throws Exception {
        GeneralAccount account = testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties);
        GeneralFileSystem fileSystem = FileFactory.newFileSystem(account);
        String targetCollection = testingPropertiesHelper.buildIRODSCollectionRelativePathFromTestProperties(testingProperties,
                IRODS_TEST_SUBDIR_PATH);
        String targetFileName = "testCopyToGeneralFileUsingSmallSourceFile.txt";

        GeneralFile file = FileFactory.newFile(fileSystem, targetCollection);

        String fullPathToLocalFile = FileGenerator.generateFileOfFixedLengthGivenName(testingProperties.getProperty(GENERATED_FILE_DIRECTORY_KEY), targetFileName, (1 * 1024));
        GeneralFile localFile = new LocalFile(fullPathToLocalFile);

        if (localFile.canRead()) {
            GeneralFile remoteFile = FileFactory.newFile(file,
                    localFile.getName());
            localFile.copyTo(remoteFile, true);
        }

        // now check if file exists in irods
        // TODO: violates DRY, create an AssertHelper class with stuff like this below
        IlsCommand ilsCommand = new IlsCommand();
        targetCollection = testingPropertiesHelper.buildIRODSCollectionAbsolutePathFromTestProperties(testingProperties,
                IRODS_TEST_SUBDIR_PATH);
        ilsCommand.setIlsBasePath(targetCollection);
        IrodsInvocationContext invocationContext = testingPropertiesHelper.buildIRODSInvocationContextFromTestProperties(testingProperties);
        IcommandInvoker invoker = new IcommandInvoker(invocationContext);
        String res = invoker.invokeCommandAndGetResultAsString(ilsCommand);
        TestCase.assertTrue("did not find file I just put",
            res.indexOf(targetFileName) > -1);
    }
    
   
    /**
     * Test method for
     * {@link edu.sdsc.grid.io.local.LocalFile#copyTo(edu.sdsc.grid.io.GeneralFile, boolean)}
     * .
     */
    @Ignore
    public final void testCopyToGeneralFileBoolean() {
        fail("Not yet implemented");
    }

    /**
     * Test method for
     * {@link edu.sdsc.grid.io.local.LocalFile#copyFrom(edu.sdsc.grid.io.GeneralFile)}
     * .
     */
    @Ignore
    public final void testCopyFromGeneralFile() {
        fail("Not yet implemented");
    }

    /**
     * Test method for
     * {@link edu.sdsc.grid.io.local.LocalFile#copyFrom(edu.sdsc.grid.io.GeneralFile, boolean)}
     * .
     */
    @Ignore
    public final void testCopyFromGeneralFileBoolean() {
        fail("Not yet implemented");
    }

    /**
     * Test method for {@link edu.sdsc.grid.io.local.LocalFile#canRead()}.
     */
    @Ignore
    public final void testCanRead() {
        fail("Not yet implemented");
    }

    /**
     * Test method for {@link edu.sdsc.grid.io.local.LocalFile#canWrite()}.
     */
    @Ignore
    public final void testCanWrite() {
        fail("Not yet implemented");
    }

    /**
     * Test method for
     * {@link edu.sdsc.grid.io.local.LocalFile#compareTo(edu.sdsc.grid.io.GeneralFile)}
     * .
     */
    @Ignore
    public final void testCompareToGeneralFile() {
        fail("Not yet implemented");
    }

    /**
     * Test method for
     * {@link edu.sdsc.grid.io.local.LocalFile#compareTo(java.lang.Object)}.
     */
    @Ignore
    public final void testCompareToObject() {
        fail("Not yet implemented");
    }

    /**
     * Test method for {@link edu.sdsc.grid.io.local.LocalFile#createNewFile()}.
     */
    @Ignore
    public final void testCreateNewFile() {
        fail("Not yet implemented");
    }

    /**
     * Test method for
     * {@link edu.sdsc.grid.io.local.LocalFile#createTempFile(java.lang.String, java.lang.String)}
     * .
     */
    @Ignore
    public final void testCreateTempFileStringString() {
        fail("Not yet implemented");
    }

    /**
     * Test method for
     * {@link edu.sdsc.grid.io.local.LocalFile#createTempFile(java.lang.String, java.lang.String, edu.sdsc.grid.io.GeneralFile)}
     * .
     */
    @Ignore
    public final void testCreateTempFileStringStringGeneralFile() {
        fail("Not yet implemented");
    }

    /**
     * Test method for {@link edu.sdsc.grid.io.local.LocalFile#delete()}.
     */
    @Ignore
    public final void testDelete() {
        fail("Not yet implemented");
    }

    /**
     * Test method for {@link edu.sdsc.grid.io.local.LocalFile#deleteOnExit()}.
     */
    @Ignore
    public final void testDeleteOnExit() {
        fail("Not yet implemented");
    }

    /**
     * Test method for
     * {@link edu.sdsc.grid.io.local.LocalFile#equals(java.lang.Object)}.
     */
    @Ignore
    public final void testEqualsObject() {
        fail("Not yet implemented");
    }

    /**
     * Test method for {@link edu.sdsc.grid.io.local.LocalFile#exists()}.
     */
    @Ignore
    public final void testExists() {
        fail("Not yet implemented");
    }

    /**
     * Test method for
     * {@link edu.sdsc.grid.io.local.LocalFile#getAbsoluteFile()}.
     */
    @Ignore
    public final void testGetAbsoluteFile() {
        fail("Not yet implemented");
    }

    /**
     * Test method for
     * {@link edu.sdsc.grid.io.local.LocalFile#getAbsolutePath()}.
     */
    @Ignore
    public final void testGetAbsolutePath() {
        fail("Not yet implemented");
    }

    /**
     * Test method for
     * {@link edu.sdsc.grid.io.local.LocalFile#getCanonicalFile()}.
     */
    @Ignore
    public final void testGetCanonicalFile() {
        fail("Not yet implemented");
    }

    /**
     * Test method for
     * {@link edu.sdsc.grid.io.local.LocalFile#getCanonicalPath()}.
     */
    @Ignore
    public final void testGetCanonicalPath() {
        fail("Not yet implemented");
    }

    /**
     * Test method for {@link edu.sdsc.grid.io.local.LocalFile#getName()}.
     */
    @Ignore
    public final void testGetName() {
        fail("Not yet implemented");
    }

    /**
     * Test method for {@link edu.sdsc.grid.io.local.LocalFile#getParent()}.
     */
    @Ignore
    public final void testGetParent() {
        fail("Not yet implemented");
    }

    /**
     * Test method for {@link edu.sdsc.grid.io.local.LocalFile#getParentFile()}.
     */
    @Ignore
    public final void testGetParentFile() {
        fail("Not yet implemented");
    }

    /**
     * Test method for {@link edu.sdsc.grid.io.local.LocalFile#getPath()}.
     */
    @Ignore
    public final void testGetPath() {
        fail("Not yet implemented");
    }

    /**
     * Test method for {@link edu.sdsc.grid.io.local.LocalFile#isAbsolute()}.
     */
    @Ignore
    public final void testIsAbsolute() {
        fail("Not yet implemented");
    }

    /**
     * Test method for {@link edu.sdsc.grid.io.local.LocalFile#isDirectory()}.
     */
    @Ignore
    public final void testIsDirectory() {
        fail("Not yet implemented");
    }

    /**
     * Test method for {@link edu.sdsc.grid.io.local.LocalFile#isFile()}.
     */
    @Ignore
    public final void testIsFile() {
        fail("Not yet implemented");
    }

    /**
     * Test method for {@link edu.sdsc.grid.io.local.LocalFile#isHidden()}.
     */
    @Ignore
    public final void testIsHidden() {
        fail("Not yet implemented");
    }

    /**
     * Test method for {@link edu.sdsc.grid.io.local.LocalFile#lastModified()}.
     */
    @Ignore
    public final void testLastModified() {
        fail("Not yet implemented");
    }

    /**
     * Test method for {@link edu.sdsc.grid.io.local.LocalFile#length()}.
     */
    @Ignore
    public final void testLength() {
        fail("Not yet implemented");
    }

    /**
     * Test method for {@link edu.sdsc.grid.io.local.LocalFile#list()}.
     */
    @Ignore
    public final void testList() {
        fail("Not yet implemented");
    }

    /**
     * Test method for {@link edu.sdsc.grid.io.local.LocalFile#listFiles()}.
     */
    @Ignore
    public final void testListFiles() {
        fail("Not yet implemented");
    }

    /**
     * Test method for {@link edu.sdsc.grid.io.local.LocalFile#listRoots()}.
     */
    @Ignore
    public final void testListRoots() {
        fail("Not yet implemented");
    }

    /**
     * Test method for {@link edu.sdsc.grid.io.local.LocalFile#mkdir()}.
     */
    @Ignore
    public final void testMkdir() {
        fail("Not yet implemented");
    }

    /**
     * Test method for {@link edu.sdsc.grid.io.local.LocalFile#mkdirs()}.
     */
    @Ignore
    public final void testMkdirs() {
        fail("Not yet implemented");
    }

    /**
     * Test method for
     * {@link edu.sdsc.grid.io.local.LocalFile#renameTo(edu.sdsc.grid.io.GeneralFile)}
     * .
     */
    @Ignore
    public final void testRenameTo() {
        fail("Not yet implemented");
    }

    /**
     * Test method for
     * {@link edu.sdsc.grid.io.local.LocalFile#setLastModified(long)}.
     */
    @Ignore
    public final void testSetLastModified() {
        fail("Not yet implemented");
    }

    /**
     * Test method for {@link edu.sdsc.grid.io.local.LocalFile#setReadOnly()}.
     */
    @Ignore
    public final void testSetReadOnly() {
        fail("Not yet implemented");
    }

    /**
     * Test method for {@link edu.sdsc.grid.io.local.LocalFile#toString()}.
     */
    @Ignore
    public final void testToString() {
        fail("Not yet implemented");
    }

    /**
     * Test method for {@link edu.sdsc.grid.io.local.LocalFile#toURI()}.
     */
    @Ignore
    public final void testToURI() {
        fail("Not yet implemented");
    }

    /**
     * Test method for {@link edu.sdsc.grid.io.local.LocalFile#toURL()}.
     */
    @Ignore
    public final void testToURL() {
        fail("Not yet implemented");
    }

    /**
     * Test method for
     * {@link edu.sdsc.grid.io.local.LocalFile#LocalFile(java.lang.String)}.
     */
    @Ignore
    public final void testLocalFileString() {
        fail("Not yet implemented");
    }

    /**
     * Test method for
     * {@link edu.sdsc.grid.io.local.LocalFile#LocalFile(java.lang.String, java.lang.String)}
     * .
     */
    @Ignore
    public final void testLocalFileStringString() {
        fail("Not yet implemented");
    }

    /**
     * Test method for
     * {@link edu.sdsc.grid.io.local.LocalFile#LocalFile(edu.sdsc.grid.io.local.LocalFile, java.lang.String)}
     * .
     */
    @Ignore
    public final void testLocalFileLocalFileString() {
        fail("Not yet implemented");
    }

    /**
     * Test method for
     * {@link edu.sdsc.grid.io.local.LocalFile#LocalFile(java.io.File)}.
     */
    @Ignore
    public final void testLocalFileFile() {
        fail("Not yet implemented");
    }

    /**
     * Test method for
     * {@link edu.sdsc.grid.io.local.LocalFile#LocalFile(java.io.File, java.lang.String)}
     * .
     */
    @Ignore
    public final void testLocalFileFileString() {
        fail("Not yet implemented");
    }

    /**
     * Test method for
     * {@link edu.sdsc.grid.io.local.LocalFile#LocalFile(java.net.URI)}.
     */
    @Ignore
    public final void testLocalFileURI() {
        fail("Not yet implemented");
    }

    /**
     * Test method for {@link edu.sdsc.grid.io.local.LocalFile#listIterator()}.
     */
    @Ignore
    public final void testListIterator() {
        fail("Not yet implemented");
    }

    /**
     * Test method for {@link edu.sdsc.grid.io.local.LocalFile#getFile()}.
     */
    @Ignore
    public final void testGetFile() {
        fail("Not yet implemented");
    }

    /**
     * Test method for {@link edu.sdsc.grid.io.local.LocalFile#getMetaData()}.
     */
    @Ignore
    public final void testGetMetaData() {
        fail("Not yet implemented");
    }

    /**
     * Test method for
     * {@link edu.sdsc.grid.io.local.LocalFile#getHomeDirectory()}.
     */
    @Ignore
    public final void testGetHomeDirectory() {
        fail("Not yet implemented");
    }

    /**
     * Test method for
     * {@link edu.sdsc.grid.io.local.LocalFile#list(java.io.FilenameFilter)}.
     */
    @Ignore
    public final void testListFilenameFilter() {
        fail("Not yet implemented");
    }

    /**
     * Test method for
     * {@link edu.sdsc.grid.io.local.LocalFile#listFiles(java.io.FileFilter)}.
     */
    @Ignore
    public final void testListFilesFileFilter() {
        fail("Not yet implemented");
    }

    /**
     * Test method for
     * {@link edu.sdsc.grid.io.local.LocalFile#listFiles(java.io.FilenameFilter)}
     * .
     */
    @Ignore
    public final void testListFilesFilenameFilter() {
        fail("Not yet implemented");
    }
}
