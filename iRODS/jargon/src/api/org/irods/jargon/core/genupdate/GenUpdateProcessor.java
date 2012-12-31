/**
 * 
 */
package org.irods.jargon.core.genupdate;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import org.irods.jargon.core.exception.JargonException;
import org.irods.jargon.core.packinstr.GenUpdateInp;
import org.irods.jargon.core.packinstr.InxVal;
import org.irods.jargon.core.query.ExtensibleMetaDataMapping;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import edu.sdsc.grid.io.irods.IRODSCommands;
import edu.sdsc.grid.io.irods.Tag;

/**
 * Process a general update request to IRODS. In the current form, this is
 * designed to be invoked by <code>IRODSCommands</code>, and should not be used
 * outside of <code>IRODSCommands</code> in the current Jargon, as that class is
 * synchronizing the communication to IRODS.
 * 
 * This class is meant to be called by IRODSCommands, and should not be invoked
 * outside that code.
 * 
 * @author Mike Conway - DICE (www.irods.org)
 * 
 */
public final class GenUpdateProcessor {
	private final IRODSCommands irodsCommands;
	private static Logger log = LoggerFactory
			.getLogger(GenUpdateProcessor.class);

	private GenUpdateProcessor(final IRODSCommands irodsCommands)
			throws JargonException {
		if (irodsCommands == null) {
			throw new JargonException("irodsCommands are null");
		}
		this.irodsCommands = irodsCommands;
	}

	/**
	 * Return a new instance of GenUpdateProcessor
	 * 
	 * @param irodsCommands
	 *            <code>IRODSCommands</code> object that will handle
	 *            communication with IRODS.
	 * @return <code>GenUpdateProcessor</code>
	 * @throws JargonException
	 */
	public static GenUpdateProcessor instance(final IRODSCommands irodsCommands)
			throws JargonException {
		return new GenUpdateProcessor(irodsCommands);
	}

	public void processGeneralUpdateOrInsert(
			final List<GenUpdateFieldValuePair> updateValues)
			throws JargonException {
		if (updateValues == null || updateValues.size() == 0) {
			throw new JargonException("update values are null or empty");
		}

		List<GenUpdateFieldValuePair> updateValuesLocal = Collections
				.unmodifiableList(updateValues);

		log.info("processing a general update, getting metadata mapping");

		// convert the field 'names' to configured numeric values to build the
		// XML protocol request
		ExtensibleMetaDataMapping mapping = ExtensibleMetaDataMapping
				.instance();
		List<InxVal> mappingValues = new ArrayList<InxVal>(
				updateValuesLocal.size());
		InxVal inxVal = null;
		String translatedName = null;

		for (GenUpdateFieldValuePair fvp : updateValuesLocal) {
			translatedName = mapping.getIndexFromColumnName(fvp.getName());

			if (translatedName == null) {
				String msg = "translated name was null for:" + fvp.getName();
				log.error(msg);
				throw new JargonException(msg);
			}

			if (log.isDebugEnabled()) {
				log.debug("translation for:" + fvp.getName() + " is:"
						+ translatedName);
			}

			inxVal = InxVal.instance(new Integer(translatedName),
					fvp.getValue());

			mappingValues.add(inxVal);

		}

		// build the general update tag
		GenUpdateInp genUpdateInp = GenUpdateInp.instance(
				GenUpdateInp.UpdateType.INSERT, mappingValues);
		try {
			Tag response = irodsCommands.irodsFunction(GenUpdateInp.PI_TAG,
					genUpdateInp.getTagValue(), GenUpdateInp.API_NBR);
			if (log.isDebugEnabled()) {
				log.debug("response from general update:" + response.parseTag());
			}
		} catch (IOException e) {
			String msg = ("IOException error processing general update through icommands, rethrown as JargonException");
			e.printStackTrace();
			log.error(msg, e);
			throw new JargonException(e);
		}
	}

	public void processGeneralDelete() throws JargonException {

	}

}
