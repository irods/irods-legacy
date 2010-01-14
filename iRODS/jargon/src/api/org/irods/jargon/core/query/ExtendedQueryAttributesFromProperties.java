/**
 * 
 */
package org.irods.jargon.core.query;

import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.List;
import java.util.Properties;

import org.irods.jargon.core.exception.JargonException;


/**
 * Implementation of {@link org.irods.jargon.core.query.IcatExtendedQueryAttributes IcatExtendedQueryAttributes} 
 * that will automatically derive the attribute mapping from a known properties file, teh extended_icat_data.properties
 * file which needs to be in the classpath.
 * 
 * This class is thread-safe, but must not expose the underlying <code>List<code> of attributes.  The <code>List</code> will
 * contain immutable value objects.
 * 
 * @author Mike Conway - DICE (www.irods.org)
 * @since 2.3
 */
public final class ExtendedQueryAttributesFromProperties extends
		IcatExendedQueryAttributes {
	
	public static final String 	ICAT_PROPERTIES_FILE = "extended_icat_data.properties";
	
	public static ExtendedQueryAttributesFromProperties instance() throws JargonException {
		return new ExtendedQueryAttributesFromProperties();
	}
	
	private ExtendedQueryAttributesFromProperties() throws JargonException {
		this.setExtendedQueryAttributes(initializeAttributesFromProperties());
	}
	
	private List<ExtendedQueryAttribute> initializeAttributesFromProperties() throws JargonException {
		// find the properties file
		ClassLoader loader = this.getClass().getClassLoader();
		InputStream in = loader.getResourceAsStream(ICAT_PROPERTIES_FILE);
		if (in == null) {
			throw new JargonException("could not find the properties file:" + ICAT_PROPERTIES_FILE);
		}
		
		Properties properties = new Properties();
		List<ExtendedQueryAttribute> tempList = new ArrayList<ExtendedQueryAttribute>();

		try {
			properties.load(in);
		} catch (IOException ioe) {
			throw new JargonException("error loading " + ICAT_PROPERTIES_FILE + " which needs to be in the classpath",
					ioe);
		} finally {
			try {
				in.close();
			} catch (Exception e) {
				// ignore
			}
		}
		
		// go through the properties and initialize the List
		String key;
		String[] tableAndColumn;
		ExtendedQueryAttribute extendedQueryAttribute;
		for (Enumeration<Object> e = properties.keys(); e.hasMoreElements();) {
			key = (String) e.nextElement();
			tableAndColumn=key.split("\\.");
			if(tableAndColumn.length != 2) {
				throw new JargonException("invalid property key, must be tablename.columnname, was:" + key);
			}
			
			String attribValue = properties.getProperty(key);
			extendedQueryAttribute = ExtendedQueryAttribute.instance(tableAndColumn[0], tableAndColumn[1], attribValue);
			tempList.add(extendedQueryAttribute);
		}
		return tempList;
	}
	
}
