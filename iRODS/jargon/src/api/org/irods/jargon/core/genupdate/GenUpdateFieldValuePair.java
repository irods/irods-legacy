/**
 * 
 */
package org.irods.jargon.core.genupdate;

import org.irods.jargon.core.exception.JargonException;

/**
 * @author Mike Conway - DICE (www.irods.org)
 * Wraps a name/value pair for a GeneralUpdate request as an immutable object
 */
public final class GenUpdateFieldValuePair {
	private final String name;
	private final String value;
	
	public static GenUpdateFieldValuePair instance(String name, String value) throws JargonException {
		return new GenUpdateFieldValuePair(name, value);
	}
	
	private GenUpdateFieldValuePair(String name, String value) throws JargonException {
		if (name == null || name.length() == 0) {
			throw new JargonException("name is null or blank");
		}
		
		if (value == null) {
			throw new JargonException("value is null");
		}
		
		this.name = name;
		this.value = value;
	}

	public String getName() {
		return name;
	}

	public String getValue() {
		return value;
	}
	
}
