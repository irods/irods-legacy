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
 * Translation of a CollInp_PI operation into XML protocol format. Object is
 * immutable an thread-safe.
 * 
 * @author Mike Conway - DICE (www.irods.org)
 * 
 */
public class CollInp extends AbstractIRODSPackingInstruction {

	public static final String PI_TAG = "CollInpNew_PI";
	public static final String COLL_NAME = "collName";
	public static final String RECURSIVE_OPR = "recursiveOpr";
	public static final String FORCE_FLAG = "forceFlag";
	public static final String FLAGS = "flags";
	public static final String OPR_TYPE = "oprType";
	public static final int MKDIR_API_NBR = 681;
	public static final int RMDIR_API_NBR = 679;
	public static final boolean RECURSIVE_OPERATION = true;
	public static final boolean NON_RECURSIVE_OPERATION = false;
	public static final boolean FORCE_OPERATION = true;

	public enum ApiOperation {
		CREATE_NEW
	}

	private static Logger log = LoggerFactory.getLogger(CollInp.class);

	private final String collectionName;
	private final boolean recursiveOperation;
	private final boolean forceOperation;

	public static final CollInp instance(String collectionName,
			boolean recursiveOperation) throws JargonException {
		return new CollInp(collectionName, recursiveOperation);
	}
	
	public static final CollInp instance(String collectionName,
			boolean recursiveOperation, boolean forceOperation) throws JargonException {
		return new CollInp(collectionName, recursiveOperation, forceOperation);
	}

	private CollInp(String collectionName, boolean recursiveOperation)
			throws JargonException {
		super();
		if (collectionName == null || collectionName.length() == 0) {
			throw new JargonException("collection name is null or blank");
		}
		this.collectionName = collectionName;
		this.recursiveOperation = recursiveOperation;
		this.forceOperation = false;
	}

	private CollInp(String collectionName, boolean recursiveOperation,
			boolean forceOperation) throws JargonException {
		super();
		if (collectionName == null || collectionName.length() == 0) {
			throw new JargonException("collection name is null or blank");
		}
		this.collectionName = collectionName;
		this.recursiveOperation = recursiveOperation;
		this.forceOperation = forceOperation;
		
	}

	public String getCollectionName() {
		return collectionName;
	}

	public boolean isRecursiveOperation() {
		return recursiveOperation;
	}

	public boolean isForceOperation() {
		return forceOperation;
	}

	@Override
	public Tag getTagValue() throws JargonException {
		Tag message = new Tag(PI_TAG, new Tag[] {
				new Tag(COLL_NAME, getCollectionName()), new Tag(FLAGS, 0),
				new Tag(OPR_TYPE, 0) });

		List<KeyValuePair> kvps = new ArrayList<KeyValuePair>();

		if (this.isForceOperation()) {
			kvps.add(KeyValuePair.instance(FORCE_FLAG, ""));
		}
		
		if (this.isRecursiveOperation()) {
			kvps.add(KeyValuePair.instance(RECURSIVE_OPR, ""));
		}

		message.addTag(createKeyValueTag(kvps));
		return message;
	}

	

}
