/**
 * 
 */
package org.irods.jargon.core.query;

import static edu.sdsc.grid.io.irods.IRODSConstants.GEN_QUERY_AN;
import static edu.sdsc.grid.io.irods.IRODSConstants.GenQueryInp_PI;
import static edu.sdsc.grid.io.irods.IRODSConstants.InxIvalPair_PI;
import static edu.sdsc.grid.io.irods.IRODSConstants.InxValPair_PI;
import static edu.sdsc.grid.io.irods.IRODSConstants.RODS_API_REQ;
import static edu.sdsc.grid.io.irods.IRODSConstants.attriCnt;
import static edu.sdsc.grid.io.irods.IRODSConstants.attriInx;
import static edu.sdsc.grid.io.irods.IRODSConstants.continueInx;
import static edu.sdsc.grid.io.irods.IRODSConstants.iiLen;
import static edu.sdsc.grid.io.irods.IRODSConstants.inx;
import static edu.sdsc.grid.io.irods.IRODSConstants.isLen;
import static edu.sdsc.grid.io.irods.IRODSConstants.ivalue;
import static edu.sdsc.grid.io.irods.IRODSConstants.maxRows;
import static edu.sdsc.grid.io.irods.IRODSConstants.options;
import static edu.sdsc.grid.io.irods.IRODSConstants.partialStartIndex;
import static edu.sdsc.grid.io.irods.IRODSConstants.rowCnt;
import static edu.sdsc.grid.io.irods.IRODSConstants.svalue;

import java.io.IOException;
import java.lang.reflect.Array;
import java.util.ArrayList;
import java.util.List;
import java.util.Vector;

import org.irods.jargon.core.exception.JargonException;
import org.irods.jargon.core.exception.JargonRuntimeException;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import edu.sdsc.grid.io.MetaDataCondition;
import edu.sdsc.grid.io.MetaDataField;
import edu.sdsc.grid.io.MetaDataRecordList;
import edu.sdsc.grid.io.MetaDataSelect;
import edu.sdsc.grid.io.MetaDataSet;
import edu.sdsc.grid.io.Namespace;
import edu.sdsc.grid.io.irods.IRODSCommands;
import edu.sdsc.grid.io.irods.IRODSMetaDataConditionWrapper;
import edu.sdsc.grid.io.irods.IRODSMetaDataRecordList;
import edu.sdsc.grid.io.irods.IRODSMetaDataSelectWrapper;
import edu.sdsc.grid.io.irods.IRODSMetaDataSet;
import edu.sdsc.grid.io.irods.Tag;

/**
 * Mid level service to process a GenQuery.  This service supports the traditional Jargon queries using MetaDataSelect and MetaDataCondition.
 * @author Mike Conway - DICE (www.irods.org)
 *
 */
public class GenQueryClassicMidLevelService {
	
	/**
	 * Removes null and duplicate values from an array.
	 */
	static final Object[] cleanNullsAndDuplicates(Object[] obj) {
		if (obj == null)
			return null;

		Vector<Object> temp = new Vector<Object>(obj.length);
		boolean anyAdd = false;
		int i = 0, j = 0;

		for (i = 0; i < obj.length; i++) {
			if (obj[i] != null) {
				// need to keep them in original order
				// keep the first, remove the rest.
				for (j = i + 1; j < obj.length; j++) {
					if (obj[i].equals(obj[j])) {
						obj[j] = null;
						j = obj.length;
					}
				}

				if (obj[i] != null) {
					temp.add(obj[i]);
					if (!anyAdd)
						anyAdd = true;
				}
			}
		}
		if (!anyAdd)
			return null;

		// needs its own check
		if ((obj.length == 1) && (obj[0] == null)) {
			return null;
		}

		return temp.toArray((Object[]) Array.newInstance(
				temp.get(0).getClass(), 0));
	}
	public static GenQueryClassicMidLevelService instance(final IRODSCommands irodsCommands) throws JargonException {
		return new GenQueryClassicMidLevelService(irodsCommands);
	}

	private final IRODSCommands irodsCommands;
	
	private Logger log = LoggerFactory.getLogger(this.getClass());
	
	private GenQueryClassicMidLevelService(final IRODSCommands irodsCommands) throws JargonException {
		if (irodsCommands == null) {
			String msg ="irodsCommands is null";
			log.error(msg);
			throw new JargonException(msg);
		}
		
		if (!irodsCommands.isConnected()) {
			String msg = "irodsCommands is not connected";
			log.error(msg);
			throw new JargonException(msg);
		}
		
		this.irodsCommands = irodsCommands;
		
	}
	
	/**
	 * @param namespace
	 * @param translatedSelects
	 * @param avuQueryAddedForNamespace
	 * @return
	 * @throws JargonRuntimeException
	 */
	private List<IRODSMetaDataSelectWrapper> amendQuerySelectsWithTranslations(
			Namespace namespace,
			List<IRODSMetaDataSelectWrapper> translatedSelects,
			boolean avuQueryAddedForNamespace) throws JargonRuntimeException {
		String translatedField;
		String originalField;
		List<IRODSMetaDataSelectWrapper> amendedSelects = new ArrayList<IRODSMetaDataSelectWrapper>();

		for (IRODSMetaDataSelectWrapper currentSelect : translatedSelects) {
			originalField = currentSelect.getOriginalMetaDataSelect()
					.getFieldName();

			if (originalField == null) {
				throw new JargonRuntimeException(
						"original meta data field is null");
			}

			if (log.isDebugEnabled()) {
				log.debug("translating select field:" + originalField);
			}

			// process each of the selects to see if this is an irods gen query
			// column

			translatedField = IRODSMetaDataSet.getID(originalField);

			if (log.isDebugEnabled()) {
				log.debug("tried translating as irods meta data and got:"
						+ translatedField);
			}

			if (!translatedField.equals(originalField)) {
				log.debug("hit on irods gen query type");
				// if translated field is different, then the lookup was a hit
				currentSelect.setTranslatedMetaDataNumber(translatedField);
				currentSelect
						.setSelectType(IRODSMetaDataSelectWrapper.SelectType.IRODS_GEN_QUERY_METADATA);
				amendedSelects.add(currentSelect);
				continue;
			}

			// no hit, see if this is extensible meta data

			log
					.debug("no translation found yet, check as extensible meta data");
			translatedField = IRODSMetaDataSet
					.getIDFromExtensibleMetaData(originalField);

			if (translatedField != null) {
				log.debug("hit on extensible metadata type");
				currentSelect.setTranslatedMetaDataNumber(translatedField);
				currentSelect
						.setSelectType(IRODSMetaDataSelectWrapper.SelectType.EXTENSIBLE_METADATA);
				amendedSelects.add(currentSelect);
				continue;
			}

			/*
			 * treat this as an AVU type. Add a query for the attribute and
			 * value that goes with the namespace, do this one time only so as
			 * not to create duplicate selects
			 */

			log
					.debug("treat as an AVU, see if I've already added a select for the attribute and name for the namespace");

			if (avuQueryAddedForNamespace) {
				log
						.debug("avu name and attribute query already added, skipping select generation");
			} else {
				log
						.debug("adding avu name and attribute select for this namespace");
				MetaDataSelect avuSelect = MetaDataSet
						.newSelection(IRODSAvu.getAttributeName(namespace));
				IRODSMetaDataSelectWrapper avuWrapper;
				try {
					avuWrapper = new IRODSMetaDataSelectWrapper(avuSelect);
				} catch (JargonException e) {
					e.printStackTrace();
					log.error("exception creating an AVU select", e);
					throw new JargonRuntimeException(e);
				}
				avuWrapper
						.setSelectType(IRODSMetaDataSelectWrapper.SelectType.AVU_METADATA);
				avuWrapper.setTranslatedMetaDataNumber(IRODSAvu
						.getAttributeNumericName(namespace).toString());
				amendedSelects.add(avuWrapper);

				if (log.isDebugEnabled()) {
					log.debug("generated a select for avu name:" + avuWrapper);
				}

				avuSelect = MetaDataSet.newSelection(IRODSAvu
						.getAttributeValue(namespace));
				try {
					avuWrapper = new IRODSMetaDataSelectWrapper(avuSelect);
				} catch (JargonException e) {
					e.printStackTrace();
					log.error("exception creating an AVU select", e);
					throw new JargonRuntimeException(e);
				}
				avuWrapper
						.setSelectType(IRODSMetaDataSelectWrapper.SelectType.AVU_METADATA);
				avuWrapper.setTranslatedMetaDataNumber(IRODSAvu
						.getAttributeNumericValue(namespace).toString());
				amendedSelects.add(avuWrapper);

				if (log.isDebugEnabled()) {
					log.debug("generated a select for avu value:" + avuWrapper);
				}

				// don't generate again
				avuQueryAddedForNamespace = true;
			}

		}
		return amendedSelects;
	}
	
	/**
	 * @param namespace
	 * @param derivedMetaDataConditions
	 * @return
	 * @throws JargonRuntimeException
	 */
	private List<IRODSMetaDataConditionWrapper> amendQueryWithAVUSelects(
			Namespace namespace,
			List<IRODSMetaDataConditionWrapper> derivedMetaDataConditions)
			throws JargonRuntimeException {
		List<IRODSMetaDataConditionWrapper> amendedConditions = new ArrayList<IRODSMetaDataConditionWrapper>();

		for (IRODSMetaDataConditionWrapper currentCondition : derivedMetaDataConditions) {
			if (currentCondition.getSelectType() == IRODSMetaDataConditionWrapper.SelectType.IRODS_GEN_QUERY_METADATA) {
				if (log.isDebugEnabled()) {
					log
							.debug("irods gen query data condition, move as is to amended:"
									+ currentCondition);
				}
				amendedConditions.add(currentCondition);
			} else if (currentCondition.getSelectType() == IRODSMetaDataConditionWrapper.SelectType.EXTENSIBLE_METADATA) {
				if (log.isDebugEnabled()) {
					log
							.debug("extensible metadata condition, move as is to amended:"
									+ currentCondition);
				}
				amendedConditions.add(currentCondition);
			} else if (currentCondition.getSelectType() == IRODSMetaDataConditionWrapper.SelectType.AVU_METADATA) {
				if (log.isDebugEnabled()) {
					log
							.debug("AVU condition, will break out the AVU conditions:"
									+ currentCondition);
				}

				// add a condition for the attribute value
				MetaDataCondition avuMetaDataCondition = MetaDataSet
						.newCondition(IRODSAvu.getAttributeValue(namespace),
								currentCondition.getMetaDataCondition()
										.getOperator(), currentCondition
										.getMetaDataCondition()
										.getStringValue());
				IRODSMetaDataConditionWrapper newConditionWrapper;
				try {
					newConditionWrapper = new IRODSMetaDataConditionWrapper(
							avuMetaDataCondition);
				} catch (JargonException e) {
					e.printStackTrace();
					log.error("exception translating a condition field", e);
					throw new JargonRuntimeException(e);
				}
				newConditionWrapper
						.setAvuComponent(IRODSMetaDataConditionWrapper.AVUComponent.ATTRIB_VALUE_COMPONENT);
				newConditionWrapper
						.setSelectType(IRODSMetaDataConditionWrapper.SelectType.AVU_METADATA);
				newConditionWrapper.setTranslatedMetaDataNumber(IRODSAvu
						.getAttributeNumericValue(namespace).toString());
				amendedConditions.add(newConditionWrapper);

				if (log.isDebugEnabled()) {
					log.debug("added a condition for the AVU attribute value:"
							+ newConditionWrapper);
				}

				// add a condition for the attribute name
				avuMetaDataCondition = MetaDataSet.newCondition(IRODSAvu
						.getAttributeName(namespace), MetaDataCondition.EQUAL,
						currentCondition.getMetaDataCondition().getFieldName());
				try {
					newConditionWrapper = new IRODSMetaDataConditionWrapper(
							avuMetaDataCondition);
				} catch (JargonException e) {
					log.error("exception translating a condition field", e);
					throw new JargonRuntimeException(e);
				}
				newConditionWrapper
						.setAvuComponent(IRODSMetaDataConditionWrapper.AVUComponent.ATTRIB_NAME_COMPONENT);
				newConditionWrapper
						.setSelectType(IRODSMetaDataConditionWrapper.SelectType.AVU_METADATA);
				newConditionWrapper.setTranslatedMetaDataNumber(IRODSAvu
						.getAttributeNumericName(namespace).toString());
				amendedConditions.add(newConditionWrapper);

				if (log.isDebugEnabled()) {
					log.debug("added a condition for the AVU attribute name:"
							+ newConditionWrapper);
				}

			}
		}
		
		// now that I have the amended conditions, I need to make sure the AVU data is in the proper order to avoid pathological SQL
		List<IRODSMetaDataConditionWrapper> amendedAndReorderedConditions = new ArrayList<IRODSMetaDataConditionWrapper>();

		log.debug("reordering amended conditions to put AVU data in the right order");
		// first, put in any non-avu query items
		for (IRODSMetaDataConditionWrapper amendedCondition : amendedConditions) {
			if (amendedCondition.getSelectType().equals(IRODSMetaDataConditionWrapper.SelectType.AVU_METADATA)) {
				log.debug("skip for now as avu metadata:{}", amendedCondition);
			} else {
				log.debug("not avu, go ahead and move to reordered condition:{}", amendedCondition);
				amendedAndReorderedConditions.add(amendedCondition);
			}
		}
		
		// now I've done the non-AVU conditions, do AVU values
		for (IRODSMetaDataConditionWrapper amendedCondition : amendedConditions) {
			if (amendedCondition.getSelectType().equals(IRODSMetaDataConditionWrapper.SelectType.AVU_METADATA)) {
				if (amendedCondition.getAvuComponent().equals(IRODSMetaDataConditionWrapper.AVUComponent.ATTRIB_VALUE_COMPONENT)) {
					log.debug("added attrib value component:{}", amendedCondition);
				amendedAndReorderedConditions.add(amendedCondition);
				}
			} else {
				log.debug("already processed:{}", amendedCondition);

			}
		}
		
		// now I've done the non-AVU conditions, do AVU attrib names
		for (IRODSMetaDataConditionWrapper amendedCondition : amendedConditions) {
			if (amendedCondition.getSelectType().equals(IRODSMetaDataConditionWrapper.SelectType.AVU_METADATA)) {
				if (amendedCondition.getAvuComponent().equals(IRODSMetaDataConditionWrapper.AVUComponent.ATTRIB_NAME_COMPONENT)) {
					log.debug("added attrib name component:{}", amendedCondition);
				amendedAndReorderedConditions.add(amendedCondition);
				}
			} else {
				log.debug("already processed:{}", amendedCondition);
			}
		}
		
		// control balance on counts
		if (amendedConditions.size() != amendedAndReorderedConditions.size()) {
			throw new JargonRuntimeException("original conditions and reordered conditions counts are out of balance");
		}
		
		// now clean out duplicates
		
		List<IRODSMetaDataConditionWrapper> uniqueConditions = new ArrayList<IRODSMetaDataConditionWrapper>();
		IRODSMetaDataConditionWrapper previousMetaDataConditionWrapper = null;
		
		for (IRODSMetaDataConditionWrapper wrapper : amendedAndReorderedConditions) {
			if (previousMetaDataConditionWrapper != null) {
				if (wrapper.getMetaDataCondition().equals(previousMetaDataConditionWrapper.getMetaDataCondition())) {
					log.debug("duplicate metaDataCondition ignored:{}", wrapper);
					continue;
				}  
			}
			uniqueConditions.add(wrapper);
			previousMetaDataConditionWrapper = wrapper;
		}
		
		return uniqueConditions;
	}

	public Tag buildQueryTag(MetaDataCondition[] conditions,
			MetaDataSelect[] selects, int numberOfRecordsWanted, int partialStart,
			Namespace namespace, boolean distinctQuery) {
		Tag message = new Tag(GenQueryInp_PI);

		message.addTag(new Tag(maxRows, numberOfRecordsWanted));
		message.addTag(new Tag(continueInx, 0));
		message.addTag(new Tag(partialStartIndex, partialStart));
		
		int versionValue = irodsCommands.getIrodsServerProperties().getRelVersion().compareTo("rods2.3");
		if (versionValue >= 0) {
			if (distinctQuery) {
				// reported version is at or after the version specified in
				// 'compareTo'
				message.addTag(new Tag(options, 0));
			} else {
				message.addTag(new Tag(options, 1));
			}
		}


		// package the selects
		if (selects == null) {
			throw new JargonRuntimeException(
					"Query must have at least one select value");
		}

		// clean up nulls and duplicates
		selects = (MetaDataSelect[]) 
				cleanNullsAndDuplicates(selects);

		// wrap the selects to assist in translating the given metadata names
		// into values understandable by IRODS

		List<IRODSMetaDataSelectWrapper> translatedSelects = wrapQuerySelectsForTranslation(selects);

		/*
		 * translate the selects 1) look for the select in the irods meta data -
		 * if not found 2) look for the select in the extensible meta data
		 * 
		 * if not found in either place, then call it an AVU
		 * 
		 * The translated field is put into a tag set for the selects
		 */

		boolean avuQueryAddedForNamespace = false;
		List<IRODSMetaDataSelectWrapper> amendedSelects = amendQuerySelectsWithTranslations(
				namespace, translatedSelects, avuQueryAddedForNamespace);

		List<IRODSMetaDataConditionWrapper> derivedMetaDataConditions = classifyQuerySelectsByType(conditions);

		// I now have the conditions and selects translated, go through the
		// conditions and
		// properly format any user definable metadata. (AVU's)

		List<IRODSMetaDataConditionWrapper> amendedConditions = amendQueryWithAVUSelects(
				namespace, derivedMetaDataConditions);

		message.addTag(Tag.createKeyValueTag(null));
		Tag[] subTags = null;
		int j = 1;

		subTags = new Tag[amendedSelects.size() * 2 + 1];
		subTags[0] = new Tag(iiLen, amendedSelects.size());

		for (IRODSMetaDataSelectWrapper currentSelect : amendedSelects) {
			subTags[j] = new Tag(inx, currentSelect
					.getTranslatedMetaDataNumber());
			j++;
		}

		for (IRODSMetaDataSelectWrapper currentSelect : amendedSelects) {
			// New for loop because they have to be in a certain order...
			subTags[j] = new Tag(ivalue, currentSelect
					.getOriginalMetaDataSelect().getOperation());
			j++;
		}

		message.addTag(new Tag(InxIvalPair_PI, subTags));

		// package the conditions
		if (amendedConditions.size() > 0) {
			// fix the conditions if there are AVU parts, also remove nulls

			subTags = new Tag[amendedConditions.size() * 2 + 1];
			subTags[0] = new Tag(isLen, amendedConditions.size());
			j = 1;
			for (IRODSMetaDataConditionWrapper tagCondition : amendedConditions) {
				subTags[j] = new Tag(inx, tagCondition
						.getTranslatedMetaDataNumber());
				j++;
			}

			for (IRODSMetaDataConditionWrapper tagCondition : amendedConditions) {
				// New for loop because they have to be in a certain order...
				subTags[j] = new Tag(svalue, " "
						+ tagCondition.getMetaDataCondition()
								.getOperatorString() + " '"
						+ tagCondition.getMetaDataCondition().getStringValue()
						+ "'");
				j++;
			}
			message.addTag(new Tag(InxValPair_PI, subTags));
		} else {
			// need this tag, just create a blank one
			message.addTag(new Tag(InxValPair_PI, new Tag(isLen, 0)));
		}

		if (log.isDebugEnabled()) {
			log.debug("query message tag:" + message.parseTag());
		}

		return message;
	}

	/**
	 * @param conditions
	 * @return
	 * @throws JargonRuntimeException
	 */
	private List<IRODSMetaDataConditionWrapper> classifyQuerySelectsByType(
			MetaDataCondition[] conditions) throws JargonRuntimeException {
		String translatedField;
		String originalField;
		// do a pre-pass to classify the conditions
		List<IRODSMetaDataConditionWrapper> derivedMetaDataConditions = new ArrayList<IRODSMetaDataConditionWrapper>();

		if (conditions != null) {
			for (int i = 0; i < conditions.length; i++) {
				try {
					derivedMetaDataConditions
							.add(new IRODSMetaDataConditionWrapper(
									conditions[i]));
				} catch (JargonException e) {
					e.printStackTrace();
					log.error("exception translating a condition field", e);
					throw new JargonRuntimeException(e);
				}
			}
		} else {
			log.debug("null conditions, skipping");
		}

		log
				.debug("conditions converted into wrappers, now resolving field names");

		for (IRODSMetaDataConditionWrapper currentCondition : derivedMetaDataConditions) {

			originalField = currentCondition.getMetaDataCondition()
					.getFieldName();
			if (log.isDebugEnabled()) {
				log.debug("translating meta data condition field:"
						+ originalField);
			}

			translatedField = IRODSMetaDataSet.getID(originalField);

			if (log.isDebugEnabled()) {
				log
						.debug("tried translating condition field as irods meta data and got:"
								+ translatedField);
			}

			if (!translatedField.equals(originalField)) {
				log.debug("hit on irods gen query type");
				// if translated field is different, then the lookup was a hit
				currentCondition.setTranslatedMetaDataNumber(translatedField);
				currentCondition
						.setSelectType(IRODSMetaDataConditionWrapper.SelectType.IRODS_GEN_QUERY_METADATA);
				continue;
			}

			// no hit, see if this is extensible meta data

			log
					.debug("no translation found yet, check as extensible meta data");
			translatedField = IRODSMetaDataSet
					.getIDFromExtensibleMetaData(originalField);

			if (translatedField != null) {
				log.debug("hit on extensible metadata type");
				currentCondition.setTranslatedMetaDataNumber(translatedField);
				currentCondition
						.setSelectType(IRODSMetaDataConditionWrapper.SelectType.EXTENSIBLE_METADATA);
				continue;
			}

			// treat as an AVU value
			currentCondition.setTranslatedMetaDataNumber(originalField);
			currentCondition
					.setSelectType(IRODSMetaDataConditionWrapper.SelectType.AVU_METADATA);
			log
					.debug("treating this as user-defined metadata and using the original field in the condition");

		}
		return derivedMetaDataConditions;
	}

	public  MetaDataRecordList[] query(MetaDataCondition[] conditions,
			MetaDataSelect[] selects, int numberOfRecordsWanted,
			Namespace namespace, boolean distinctQuery) throws JargonException, IOException {
		
		Tag message = buildQueryTag(conditions, selects, numberOfRecordsWanted, 0,
				namespace, distinctQuery);

		// send command to server
		message = irodsCommands.irodsFunction(RODS_API_REQ, message, GEN_QUERY_AN);

		if (message == null) {
			// query had no results
			return null; // TODO: return empty list instead of null?
		}

		int rows = message.getTag(rowCnt).getIntValue();
		int attributes = message.getTag(attriCnt).getIntValue();
		int continuation = message.getTag(continueInx).getIntValue();

		String[] results = new String[attributes];
		MetaDataField[] fields = new MetaDataField[attributes];
		MetaDataRecordList[] rl = new MetaDataRecordList[rows];
		int j = 0;
		for (int i = 0; i < attributes; i++) {

			fields[i] = IRODSMetaDataSet.getField(message.getTags()[4 + i].getTag(
					attriInx).getStringValue());
		}
		for (int i = 0; i < rows; i++) {
			for (j = 0; j < attributes; j++) {

				results[j] = message.getTags()[4 + j].getTags()[2 + i].getStringValue();
			}
			if (continuation > 0) {
				rl[i] = new IRODSMetaDataRecordList(irodsCommands, fields, results,
						continuation);
			} else {
				// No more results, don't bother with sending the IRODSCommand
				// object
				rl[i] = new IRODSMetaDataRecordList(null, fields, results,
						continuation);
			}
		}

		return rl;
		
	}

	public  MetaDataRecordList[] queryWithPartialStart(MetaDataCondition[] conditions,
			MetaDataSelect[] selects, int numberOfRecordsWanted, int partialStartIndex,
			Namespace namespace, boolean distinctQuery) throws JargonException, IOException {
		
		Tag message = buildQueryTag(conditions, selects, numberOfRecordsWanted, partialStartIndex,
				namespace, distinctQuery);

		// send command to server
		message = irodsCommands.irodsFunction(RODS_API_REQ, message, GEN_QUERY_AN);

		if (message == null) {
			// query had no results
			return null;
		}

		int rows = message.getTag(rowCnt).getIntValue();
		int attributes = message.getTag(attriCnt).getIntValue();
		int continuation = message.getTag(continueInx).getIntValue();

		String[] results = new String[attributes];
		MetaDataField[] fields = new MetaDataField[attributes];
		MetaDataRecordList[] rl = new MetaDataRecordList[rows];
		int j = 0;
		for (int i = 0; i < attributes; i++) {

			fields[i] = IRODSMetaDataSet.getField(message.getTags()[4 + i].getTag(
					attriInx).getStringValue());
		}
		for (int i = 0; i < rows; i++) {
			for (j = 0; j < attributes; j++) {

				results[j] = message.getTags()[4 + j].getTags()[2 + i].getStringValue();
			}
			if (continuation > 0) {
				rl[i] = new IRODSMetaDataRecordList(irodsCommands, fields, results,
						continuation);
			} else {
				// No more results, don't bother with sending the IRODSCommand
				// object
				rl[i] = new IRODSMetaDataRecordList(null, fields, results,
						continuation);
			}
		}

		return rl;
		
	}
	
	/**
	 * @param selects
	 * @return
	 * @throws JargonRuntimeException
	 */
	private List<IRODSMetaDataSelectWrapper> wrapQuerySelectsForTranslation(
			MetaDataSelect[] selects) throws JargonRuntimeException {
		log.debug("wrapping original selects for processing");
		IRODSMetaDataSelectWrapper irodsMetaDataSelectWrapper = null;
		List<IRODSMetaDataSelectWrapper> translatedSelects = new ArrayList<IRODSMetaDataSelectWrapper>();

		for (int i = 0; i < selects.length; i++) {
			try {
				irodsMetaDataSelectWrapper = new IRODSMetaDataSelectWrapper(
						selects[i]);
				translatedSelects.add(irodsMetaDataSelectWrapper);
			} catch (JargonException e) {
				String msg = "error translating an IRODS select into an IRODSMetaDataSelectWrapper";
				log.error(msg);
				throw new JargonRuntimeException(msg, e);
			}

		}
		return translatedSelects;
	}

}
