/**
 * 
 */
package org.irods.jargon.core.packinstr;

import java.util.ArrayList;
import java.util.List;

import org.irods.jargon.core.exception.JargonException;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import edu.sdsc.grid.io.irods.Tag;

/**
 * Translation of a DataObjInp operation into XML protocol format.
 * 
 * @author Mike Conway - DICE (www.irods.org)
 * 
 */
public class DataObjInp extends AbstractIRODSPackingInstruction {

	public static final String PI_TAG = "DataObjInp_PI";
	public static final String OBJ_PATH = "objPath";
	public static final String CREATE_MODE = "createMode";
	public static final String OPEN_FLAGS = "openFlags";
	public static final String DATA_SIZE = "dataSize";
	public static final String OPR_TYPE = "oprType";
	public static final String OFFSET = "offset";
	public static final String NUM_THREADS = "numThreads";
	public static final String DATA_TYPE = "dataType";
	public static final String DEST_RESC_NAME = "destRescName";

	public static final int CREATE_FILE_API_NBR = 601;
	public static final int DELETE_FILE_API_NBR = 615;
	public static final int PHYMOVE_FILE_API_NBR = 631;
	public static final int OPEN_FILE_API_NBR = 602;

	static final String DATA_TYPE_GENERIC = "generic";

	public static final int DEFAULT_OPERATION_TYPE = 0;

	public static final int RENAME_FILE_OPERATION_TYPE = 11;
	public static final int RENAME_DIRECTORY_OPERATION_TYPE = 12;
	public static final int PHYMOVE_OPERATION_TYPE = 15;

	private static Logger log = LoggerFactory.getLogger(DataObjInp.class);

	public static final int DEFAULT_CREATE_MODE = 488;

	public enum OpenFlags {
		READ, WRITE, READ_WRITE
	}

	public enum ForceOptions {
		FORCE, NO_FORCE
	}

	private String fileAbsolutePath = "";
	private int createMode = DEFAULT_CREATE_MODE;
	private OpenFlags openFlags = null;
	private long offset = 0L;
	private long dataSize = 0L;
	private int numThreads = 0;
	private String resource = "";
	private ForceOptions forceOption = ForceOptions.NO_FORCE;
	private int operationType = 0;

	public static final DataObjInp instance(final String fileAbsolutePath,
			final int createMode, final OpenFlags openFlags, final long offset,
			final long dataSize, final int numThreads, final String resource)
			throws JargonException {
		return new DataObjInp(fileAbsolutePath, createMode, openFlags, offset,
				dataSize, numThreads, resource);
	}

	public static final DataObjInp instance(final String fileAbsolutePath,
			final ForceOptions forceOption) throws JargonException {
		DataObjInp dataObjInp = new DataObjInp(fileAbsolutePath,
				DEFAULT_CREATE_MODE, OpenFlags.READ, 0L, 0L, 0, "");
		dataObjInp.forceOption = forceOption;
		dataObjInp.operationType = DEFAULT_OPERATION_TYPE;
		return dataObjInp;
	}

	public static final DataObjInp instanceForRename(
			final String fileAbsolutePath, int operationType)
			throws JargonException {

		if (operationType == DataObjInp.RENAME_DIRECTORY_OPERATION_TYPE
				|| operationType == DataObjInp.RENAME_FILE_OPERATION_TYPE) {
			// ok
		} else {
			throw new JargonException("unknown operation type:" + operationType);
		}

		DataObjInp dataObjInp = DataObjInp.instance(fileAbsolutePath,
				ForceOptions.NO_FORCE);
		dataObjInp.operationType = operationType;
		return dataObjInp;

	}

	public static final DataObjInp instanceForPhysicalMove(
			final String fileAbsolutePath, final String resource)
			throws JargonException {

		if (resource == null || resource.length() == 0) {
			throw new JargonException("null or missing destination resource");
		}

		DataObjInp dataObjInp = DataObjInp.instance(fileAbsolutePath,
				ForceOptions.NO_FORCE);
		dataObjInp.resource = resource;
		dataObjInp.operationType = PHYMOVE_OPERATION_TYPE;
		return dataObjInp;

	}

	public static final DataObjInp instanceForOpen(final String fileAbsolutePath, OpenFlags openFlags) throws JargonException {
		return new DataObjInp(fileAbsolutePath, DEFAULT_CREATE_MODE, openFlags, 0L, 0L, 0, "");
	}
	
	
	private DataObjInp(String fileAbsolutePath, int createMode,
			OpenFlags openFlags, long offset, long dataSize, int numThreads,
			String resource) throws JargonException {
		super();
		if (fileAbsolutePath == null || fileAbsolutePath.length() == 0) {
			throw new JargonException("file absolute path is null or empty");
		}

		if (dataSize < 0) {
			throw new JargonException("negative data size");
		}

		if (numThreads < 0) {
			throw new JargonException("num threads is negative");
		}

		if (offset > dataSize) {
			throw new JargonException("offset is greater than data size");
		}

		if (createMode < 0) {
			throw new JargonException("invalid create mode:" + createMode);
		}

		if (resource == null) {
			throw new JargonException(
					"resource is null, may be set to blank if not required");
		}

		if (openFlags == null) {
			throw new JargonException("null open flags");
		}

		this.fileAbsolutePath = fileAbsolutePath;
		this.createMode = createMode;
		this.openFlags = openFlags;
		this.offset = offset;
		this.dataSize = dataSize;
		this.numThreads = numThreads;
		this.resource = resource;
		this.forceOption = DataObjInp.ForceOptions.NO_FORCE;

	}

	public String getParsedTags() throws JargonException {

		Tag message = getTagValue();

		String tagOut = message.parseTag();

		if (log.isDebugEnabled()) {
			log.debug("tag created:" + tagOut);
		}

		return tagOut;

	}

	public Tag getTagValue() throws JargonException {
		int tagOpenFlags = translateOpenFlagsValue();

		Tag message = new Tag(PI_TAG, new Tag[] {
				new Tag(OBJ_PATH, getFileAbsolutePath()),
				new Tag(CREATE_MODE, getCreateMode()),
				new Tag(OPEN_FLAGS, tagOpenFlags),
				new Tag(OFFSET, getOffset()),
				new Tag(DATA_SIZE, getDataSize()),
				new Tag(NUM_THREADS, getNumThreads()),
				new Tag(OPR_TYPE, getOperationType()) });

		List<KeyValuePair> kvps = new ArrayList<KeyValuePair>();
		kvps.add(KeyValuePair.instance(DATA_TYPE, DATA_TYPE_GENERIC));

		// add a keyword tag for resource if a resource was given to the packing
		// instruction.
		if (getResource().length() > 0) {
			log.debug("adding resource information to xml call:"
					+ getResource());
			kvps.add(KeyValuePair.instance(DEST_RESC_NAME, getResource()));
		}

		message.addTag(createKeyValueTag(kvps));
		return message;
	}

	/**
	 * @return
	 * @throws JargonException
	 */
	private int translateOpenFlagsValue() throws JargonException {
		int tagOpenFlags = 0;

		if (getOpenFlags() == OpenFlags.READ) {
			tagOpenFlags = 0;
		} else if (getOpenFlags() == OpenFlags.WRITE) {
			tagOpenFlags = 1;
		} else if (getOpenFlags() == OpenFlags.READ_WRITE) {
			tagOpenFlags = 2;
		} else {
			throw new JargonException("invalid open flags:" + getOpenFlags());
		}
		return tagOpenFlags;
	}

	public String getFileAbsolutePath() {
		return fileAbsolutePath;
	}

	public int getCreateMode() {
		return createMode;
	}

	public OpenFlags getOpenFlags() {
		return openFlags;
	}

	public long getOffset() {
		return offset;
	}

	public long getDataSize() {
		return dataSize;
	}

	public int getNumThreads() {
		return numThreads;
	}

	public String getResource() {
		return resource;
	}

	public ForceOptions getForceOptions() {
		return forceOption;
	}

	public int getOperationType() {
		return operationType;
	}

}
