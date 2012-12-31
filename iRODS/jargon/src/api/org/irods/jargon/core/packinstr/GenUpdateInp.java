/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package org.irods.jargon.core.packinstr;

import java.util.Collections;
import java.util.List;

import org.irods.jargon.core.exception.JargonException;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import edu.sdsc.grid.io.irods.Tag;

/**
 * Represents the protocol for requesting gn update. The numeric codes for
 * extensible metadata are passed, along with their desired values, in an
 * InxValPair format.
 * 
 * From C code: #define GeneralUpdateInp_PI "int type; struct InxValPair_PI;"
 * refer to rcGeneralUpdate
 */
public final class GenUpdateInp extends AbstractIRODSPackingInstruction
		implements IRodsPI {

	public static final int API_NBR = 710;
	public static final String PI_TAG = "GeneralUpdateInp_PI";

	public static final String GENERAL_UPDATE_INSERT_TAG_VALUE = "23451";
	public static final String GENERAL_UPDATE_DELETE_TAG_VALUE = "23452";

	public static final String UPDATE_NEXT_SEQ_VALUE = "update_next_sequence_value";
	public static final String UPDATE_NOW_TIME = "update_now_time";

	public static final String ISLEN = "isLen";
	public static final String INX = "inx";
	public static final String INX_VAL_PAIR_PI = "InxValPair_PI";
	public static final String TYPE = "type";
	public static final String SVALUE = "svalue";

	public enum UpdateType {
		INSERT, DELETE
	}

	private final UpdateType updateType;
	private final List<InxVal> columnValues;

	private static Logger log = LoggerFactory.getLogger(GenUpdateInp.class);

	public static GenUpdateInp instance(final UpdateType updateType,
			final List<InxVal> columnValues) throws JargonException {
		return new GenUpdateInp(updateType, columnValues);
	}

	private GenUpdateInp(final UpdateType updateType,
			final List<InxVal> columnValues) throws JargonException {

		if (updateType == null) {
			throw new JargonException("update type is null");
		}

		if (columnValues == null || columnValues.size() == 0) {
			throw new JargonException("column values are null or empty");
		}

		this.updateType = updateType;
		this.columnValues = Collections.unmodifiableList(columnValues);
	}

	@Override
	public Tag getTagValue() throws JargonException {
		String sendType;
		if (updateType == UpdateType.DELETE) {
			sendType = GENERAL_UPDATE_DELETE_TAG_VALUE;
		} else if (updateType == UpdateType.INSERT) {
			sendType = GENERAL_UPDATE_INSERT_TAG_VALUE;
		} else {
			String msg = "unknown update type:" + updateType;
			log.error(msg);
			throw new JargonException(msg);
		}

		Tag message = new Tag(PI_TAG, new Tag[] {
		// operation type
		new Tag(TYPE, sendType) });

		/*
		 * subtags that follow the inx format #define InxValPair_PI
		 * "int isLen; int *inx(isLen); str *svalue[isLen];"
		 */

		Tag[] subTags = new Tag[columnValues.size() * 2 + 1];
		subTags[0] = new Tag(ISLEN, columnValues.size());

		int i = 1;

		for (InxVal inxValue : columnValues) {
			subTags[i++] = new Tag(INX, inxValue.getName());
			subTags[i++] = new Tag(SVALUE, inxValue.getValue());
		}

		message.addTag(new Tag(INX_VAL_PAIR_PI, subTags));
		return message;
	}

	@Override
	public String getParsedTags() throws JargonException {

		Tag message = getTagValue();
		String tagParsed = message.parseTag();
		if (log.isDebugEnabled()) {
			log.debug("gen update tag:" + tagParsed);
		}

		return tagParsed;
	}

	/**
	 * @return <code>UpdateType</code> enum value that indicates delete or
	 *         insert operation.
	 */
	public UpdateType getUpdateType() {
		return updateType;
	}

	public List<InxVal> getColumnValues() {
		return columnValues;
	}

}
