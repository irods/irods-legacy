/**
 * 
 */
package org.irods.jargon.core.query;

import java.util.Collections;
import java.util.List;

import org.irods.jargon.core.exception.JargonException;

/**
 * Immutable view of mapping between ICAT extended attribute identifiers, and the corresponding table/column.
 * These values need to match the extended attributes defined in IRODS in server/modules/extendedICAT.h. 
 * 
 * This is a thread safe class, as it encapsulates a <code>List</code> of {link org.irods.jargon.core.query.ExtendedQueryAttribute ExtendedQueryAttribute},
 * which is immutable.
 * 
 * @author Mike Conway - DICE (www.irods.org)
 * @since 2.3
 */
public abstract class IcatExendedQueryAttributes {
	private List<ExtendedQueryAttribute> extendedQueryAttributes;
	
	protected void setExtendedQueryAttributes(List<ExtendedQueryAttribute> extendedQueryAttributes) throws JargonException {
		if (extendedQueryAttributes == null) {
			throw new JargonException("extended query attributes cannot be null");
		}
		this.extendedQueryAttributes = Collections.unmodifiableList(extendedQueryAttributes);
	}
	
	/**
	 * @param table 
	 * @param column
	 * @return matching {@link org.irods.jargon.core.query.ExtendedQueryAttribute ExtendedQueryAttribute}, or null if not found.
	 * @throws JargonException
	 */
	public ExtendedQueryAttribute getExtendedAttributeValueByTableAndColumn(String table, String column) throws JargonException {
		if (table == null || table.length() == 0) {
			throw new JargonException("table name must be provided");
		}
		
		if (column == null || column.length() == 0) {
			throw new JargonException("column name must be provided");
		}
		
		for(ExtendedQueryAttribute attrib: extendedQueryAttributes) {
			if(attrib.isMatched(table, column))
				return attrib;
		}
		
		// not found
		return null;
	}
	
	/**
	 * @param attributeValue <code>String<code> containing the numeric value mapped to this extended ICAT table/column.
	 * @return matching {@link org.irods.jargon.core.query.ExtendedQueryAttribute ExtendedQueryAttribute}, or null if not found.
	 * @throws JargonException
	 */
	public ExtendedQueryAttribute getExtendedAttributeValueByAttributeValue(String attributeValue) throws JargonException {
		if (attributeValue == null || attributeValue.length() == 0) {
			throw new JargonException("attributeValue must be provided");
		}
		
		for(ExtendedQueryAttribute attrib: extendedQueryAttributes) {
			if(attrib.getAttributeValue().equals(attributeValue));
				return attrib;
		}
		
		// not found
		return null;
	}
	
	
}
