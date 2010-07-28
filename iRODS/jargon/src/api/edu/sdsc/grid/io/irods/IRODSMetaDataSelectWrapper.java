/**
 * 
 */
package edu.sdsc.grid.io.irods;

import org.irods.jargon.core.exception.JargonException;

import edu.sdsc.grid.io.MetaDataSelect;

/**
 * Contains information about a meta data select, along with other values used in the process of translating a select
 * into the actual GenQueryInp select value.
 * 
 * This is a transitional arrangement meant to accommodate extensible metadata, and to simplify processing of <code>IRODSMetaDataSet</code>
 * 
 * @author Mike Conway - DICE (www.irods.org)
 *
 */
public class IRODSMetaDataSelectWrapper {
	private final MetaDataSelect originalMetaDataSelect;
	private SelectType selectType = SelectType.UNTRANSLATED_METADATA;
	private String translatedMetaDataNumber = "";
	
	public enum SelectType { UNTRANSLATED_METADATA, IRODS_GEN_QUERY_METADATA, EXTENSIBLE_METADATA, AVU_METADATA }
	
	public IRODSMetaDataSelectWrapper(MetaDataSelect originalMetaDataSelect) throws JargonException {
		if (originalMetaDataSelect == null) {
			throw new JargonException("null originalMetaDataSelect");
		}
		
		this.originalMetaDataSelect = originalMetaDataSelect;
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

	public MetaDataSelect getOriginalMetaDataSelect() {
		return originalMetaDataSelect;
	}
	
	public String toString() {
		StringBuilder sb = new StringBuilder();
		sb.append("IRODSMetaDataSelectWrapper\n");
		sb.append("   original select:");
		sb.append(originalMetaDataSelect);
		sb.append("\n");
		sb.append("    type of select:");
		sb.append(selectType);
		sb.append("\n");
		sb.append("    translated value:");
		sb.append(translatedMetaDataNumber);
		return sb.toString();
	}
	
}
