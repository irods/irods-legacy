/**
 * 
 */
package edu.sdsc.grid.io.irods;

import org.irods.jargon.core.exception.JargonException;

import edu.sdsc.grid.io.MetaDataCondition;

/**
 * Contains information about a metadata condition used to translate from Jargon values to internal IRODS values.
 * This is a transitional class to accommodate extensible metadata.
 * 
 * @author Mike Conway - DICE (www.irods.org)
 *
 */
public class IRODSMetaDataConditionWrapper {
	
	private final MetaDataCondition metaDataCondition;
	private SelectType selectType = SelectType.UNTRANSLATED_METADATA;
	private String translatedMetaDataNumber = "";
	private AVUComponent avuComponent = AVUComponent.UNSET;
	
	public enum SelectType { UNTRANSLATED_METADATA, IRODS_GEN_QUERY_METADATA, EXTENSIBLE_METADATA, AVU_METADATA }
	
	public enum AVUComponent { ATTRIB_NAME_COMPONENT, ATTRIB_VALUE_COMPONENT, UNSET }
	
	public String toString() {
		StringBuilder sb = new StringBuilder();
		sb.append("IRODSMetaDataConditionWrapper:");
		sb.append("\n   metaDataCondition:");
		sb.append(metaDataCondition);
		sb.append("\n   selectType:");
		sb.append(selectType);
		sb.append("\n   translatedMetaDataNumber:");
		sb.append(translatedMetaDataNumber);
		sb.append("\n  avuComponent:");
		sb.append(avuComponent);
		return sb.toString();
	}
	
	
	
	public IRODSMetaDataConditionWrapper(final MetaDataCondition metaDataCondition) throws JargonException {
		if (metaDataCondition == null) {
			throw new JargonException("null metaDataCondition");
		}
		
		this.metaDataCondition = metaDataCondition;
	}

	public SelectType getSelectType() {
		return selectType;
	}

	public void setSelectType(SelectType selectType) {
		this.selectType = selectType;
	}

	public String getTranslatedMetaDataNumber() {
		return translatedMetaDataNumber;
	}

	public void setTranslatedMetaDataNumber(String translatedMetaDataNumber) {
		this.translatedMetaDataNumber = translatedMetaDataNumber;
	}

	public MetaDataCondition getMetaDataCondition() {
		return metaDataCondition;
	}

	public AVUComponent getAvuComponent() {
		return avuComponent;
	}

	public void setAvuComponent(AVUComponent avuComponent) {
		this.avuComponent = avuComponent;
	}

}
