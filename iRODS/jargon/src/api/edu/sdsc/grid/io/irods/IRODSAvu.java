/**
 * 
 */
package edu.sdsc.grid.io.irods;

import java.util.Vector;

import edu.sdsc.grid.io.MetaDataCondition;
import edu.sdsc.grid.io.MetaDataField;
import edu.sdsc.grid.io.MetaDataSelect;
import edu.sdsc.grid.io.MetaDataSet;
import edu.sdsc.grid.io.Namespace;

/**
 * Utility class with operations on AVU data.
 * 
 * @author Mike Conway - DICE (www.irods.org)
 * 
 */
public class IRODSAvu {

	/**
	 * Checks to see if any of these conditions are really a user definable
	 * metadata AVU. And if so convert it to something irods will understand.
	 * 
	 * @param conditions
	 * @return
	 */
	static MetaDataCondition[] checkForAVU(MetaDataCondition[] conditions,
			Namespace namespace) {
		Vector<MetaDataCondition> newConditions = null;
		for (int i = 0; i < conditions.length; i++) {

			if (checkForAVU(conditions[i].getField())) {
				if (newConditions == null) {
					newConditions = new Vector();
				}
				newConditions
						.add(MetaDataSet.newCondition(
								getAttributeValue(namespace), conditions[i]
										.getOperator(), conditions[i]
										.getStringValue()));

				conditions[i] = MetaDataSet.newCondition(
						getAttributeName(namespace), MetaDataCondition.EQUAL,
						conditions[i].getFieldName());
			}
		}

		if (newConditions != null) {
			for (MetaDataCondition c : conditions) {
				newConditions.add(c);
			}
			conditions = newConditions.toArray(conditions);
		}
		return conditions;
	}

	/**
	 * Checks the selects for user definable metadata AVU. And if so convert it
	 * to something irods will understand.
	 */
	static MetaDataSelect[] checkForAVU(MetaDataCondition[] conditions,
			MetaDataSelect[] selects, Namespace namespace, String[] selectedAVU) {
		Vector<MetaDataCondition> newConditions = null;
		Vector<MetaDataSelect> newSelects = null;
		for (int i = selects.length - 1; i >= 0; i--) {
			if (checkForAVU(selects[i].getField())) {
				if (newConditions == null) {
					newConditions = new Vector();
				}
				newConditions.add(MetaDataSet.newCondition(
						getAttributeName(namespace), MetaDataCondition.EQUAL,
						selects[i].getFieldName()));

				// store for creating the query return
				selectedAVU[i] = selects[i].getFieldName();

				if (newSelects == null) {
					newSelects = new Vector();
				}
				// get the AVU value
				selects[i] = IRODSMetaDataSet
						.newSelection(getAttributeValue(namespace));
				// get the AVU attribute
				newSelects.add(MetaDataSet
						.newSelection(getAttributeName(namespace)));
			}
		}

		if (newConditions != null) {
			// join the conditions
			if (conditions != null && conditions.length > 0) {
				for (MetaDataCondition c : conditions) {
					newConditions.add(c);
				}
				conditions = newConditions.toArray(conditions);
			} else if (conditions != null) {
				// conditions length = 0
				conditions = newConditions.toArray(conditions);
			} else {
				conditions = newConditions.toArray(new MetaDataCondition[0]);

			}

			// join the selects
			for (MetaDataSelect s : selects) {
				newSelects.add(s);
			}
			selects = newSelects.toArray(selects);
		}
		return selects;
	}

	/**
	 * Just checks the extensible value of this field to see if it was declared
	 * DEFINABLE_METADATA during the conditions creation.
	 * 
	 * @param field
	 * @return
	 */
	static boolean checkForAVU(MetaDataField field) {
		// note that the getExtensibleName(null) call below will always return
		// DEFINABLE_METADATA
		String ext = field.getExtensibleName(null);
		if ((ext != null) && ext.equals(IRODSMetaDataSet.DEFINABLE_METADATA))
			return true;

		return false;
	}

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
