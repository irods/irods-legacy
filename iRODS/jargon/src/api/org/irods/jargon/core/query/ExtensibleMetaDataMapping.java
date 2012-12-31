package org.irods.jargon.core.query;

import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

import org.irods.jargon.core.exception.JargonException;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * 
 */

/**
 * Contains an index of extensible meta-data attributes. This object is
 * thread-safe. Note that this object acts as a singleton, such that it can be
 * created with a specific mapping derived from an arbitrary location. After
 * creation, the same <code>ExtensibleMetaDataMapping</code> will be returned.
 * 
 * This arrangement is not optimal, and will be changed in later versions. It is
 * required due to multiple static initializers with circular dependencies in
 * the <code>IRODSProtocol</code> and <code>IRODSMetaDataSet</code> classes.
 * 
 * Refer to the README.TXT in the /modules subdirectory of the main IRODS
 * download. Provides bi-directional lookup of extensible meta-data.
 * 
 * @author Mike Conway - DICE (www.irods.org)
 * 
 */
public final class ExtensibleMetaDataMapping {

	private static ExtensibleMetaDataMapping extensibleMetaDataMapping = null;

	private static Logger log = LoggerFactory
			.getLogger(ExtensibleMetaDataMapping.class);

	/**
	 * Create an object to hold an immutable mapping of extensible metadata
	 * columns and values using default behavior, which is to derive mappings
	 * from a properties file. See
	 * <code>ExtensibleMetadataPropertiesSource</code>.
	 * 
	 * @return
	 * @throws JargonException
	 */
	public synchronized static ExtensibleMetaDataMapping instance()
			throws JargonException {
		if (extensibleMetaDataMapping != null) {
			log.debug("retuning cached extensibleMetaDataMapping");
			return extensibleMetaDataMapping;
		}
		log.debug("cacheing and returning extensibleMetaDataMapping using the default properties mapping");
		ExtensibleMetaDataSource extensibleMetaDataSource = new ExtensibleMetadataPropertiesSource();
		ExtensibleMetaDataMapping extensibleMetaDataMapping = extensibleMetaDataSource
				.generateExtensibleMetaDataMapping();
		return extensibleMetaDataMapping;
	}

	/**
	 * Create an object to hold a mapping of extensible metadata columns and
	 * values. Note that this method will override the stored mappings from a
	 * previous construction, therefore, the access methods are synchronized.
	 * 
	 * @param extensibleMappings
	 *            <code>Map<String,String><code> containing
	 * @return
	 * @throws JargonException
	 */
	public synchronized static ExtensibleMetaDataMapping instance(
			final Map<String, String> extensibleMappings)
			throws JargonException {
		if (extensibleMetaDataMapping != null) {
			log.warn("overriding previous extensible metadata mapping");
		}
		log.debug("cacheing and returning fresh extensibleMetaDataMapping");
		Map<String, String> copiedExtensibleMappings = new HashMap<String, String>(
				extensibleMappings);
		extensibleMetaDataMapping = new ExtensibleMetaDataMapping(
				copiedExtensibleMappings);
		return extensibleMetaDataMapping;
	}

	// Map will be wrapped immutable at construction time
	private Map<String, String> extensibleMappings = new HashMap<String, String>();

	/**
	 * Private constructor will take a provided <code>Map</code>, wrap it as
	 * immutable, and then provide thread-safe access to extensible meta data
	 * mapping.
	 * 
	 * 
	 * @param extensibleMappings
	 */
	private ExtensibleMetaDataMapping(
			final Map<String, String> extensibleMappings)
			throws JargonException {
		if (extensibleMappings == null || extensibleMappings.size() == 0) {
			throw new JargonException("null or empty extensible mappings");
		}
		this.extensibleMappings = Collections
				.unmodifiableMap(extensibleMappings);
	}

	/**
	 * For a given numeric index, get the equivalent column name.
	 * 
	 * Note that method can return <code>null</code>
	 * 
	 * @param index
	 *            <code>String</code> containing the value of the extensible
	 *            metadata numeric index.
	 * @return <code>String</code> with the extensible metaata column name, or
	 *         <code>null</code> if not found.
	 */
	public String getColumnNameFromIndex(final String index) {
		String columnName = null;
		String foundIntValue = "";
		for (String key : extensibleMappings.keySet()) {
			foundIntValue = extensibleMappings.get(key);
			if (foundIntValue.equals(index)) {
				columnName = key;
				break;
			}
		}
		return columnName;
	}

	/**
	 * Given a column name (which maps to the ext_col_names_t structure in the
	 * IRODS extendediCat.h), return the numeric value which should be sent in
	 * an IRODS query.
	 * 
	 * Note that method can return <code>null</code>
	 * 
	 * @param columnName
	 *            <code>String<code> containing the column name of the extensible metadata
	 * @return <code>String</code> containing the corresponding index, or
	 *         <code>null</code> if no match is found.
	 */
	public String getIndexFromColumnName(final String columnName) {
		String index = extensibleMappings.get(columnName);
		return index;
	}

}
