/**
 * 
 */
package org.irods.jargon.core.query;

import edu.sdsc.grid.io.Namespace;
import edu.sdsc.grid.io.irods.IRODSMetaDataSet;

/**
 * Utility class with operations on AVU data.
 * 
 * @author Mike Conway - DICE (www.irods.org)
 * 
 */
public class IRODSAvu {


	static String getAttributeName(Namespace namespace) {
		switch (namespace) {
		default:
		case FILE:
			return IRODSMetaDataSet.META_DATA_ATTR_NAME;
		case DIRECTORY:
			return IRODSMetaDataSet.META_COLL_ATTR_NAME;
		case RESOURCE:
			return IRODSMetaDataSet.META_RESOURCE_ATTR_NAME;
		case USER:
			return IRODSMetaDataSet.META_USER_ATTR_NAME;
		}
	}
	
	static Integer getAttributeNumericName(Namespace namespace) {
		switch (namespace) {
		default:
		case FILE:
			return IRODSMetaDataSet.COL_META_DATA_ATTR_NAME;
		case DIRECTORY:
			return IRODSMetaDataSet.COL_META_COLL_ATTR_NAME;
		case RESOURCE:
			return IRODSMetaDataSet.COL_META_RESC_ATTR_NAME;
		case USER:
			return IRODSMetaDataSet.COL_META_USER_ATTR_NAME;
		}
	}
	
	static Integer getAttributeNumericValue(Namespace namespace) {
		switch (namespace) {
		default:
		case FILE:
			return IRODSMetaDataSet.COL_META_DATA_ATTR_VALUE;
		case DIRECTORY:
			return IRODSMetaDataSet.COL_META_COLL_ATTR_VALUE;
		case RESOURCE:
			return IRODSMetaDataSet.COL_META_RESC_ATTR_VALUE;
		case USER:
			return IRODSMetaDataSet.COL_META_USER_ATTR_VALUE;
		}
	}

	static String getAttributeValue(Namespace namespace) {
		switch (namespace) {
		default:
		case FILE:
			return IRODSMetaDataSet.META_DATA_ATTR_VALUE;
		case DIRECTORY:
			return IRODSMetaDataSet.META_COLL_ATTR_VALUE;
		case RESOURCE:
			return IRODSMetaDataSet.META_RESOURCE_ATTR_VALUE;
		case USER:
			return IRODSMetaDataSet.META_USER_ATTR_VALUE;
		}
	}

}
