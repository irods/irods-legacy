/**
 * 
 */
package org.irods.jargon.core.pub.domain;

import org.irods.jargon.core.exception.JargonException;

/**
 * Immutable representation of an AVU metadata item
 * 
 * @author Mike Conway - DICE (www.irods.org)
 * 
 */

public final class AvuData {

	private final String attribute;
	private final String value;
	private final String unit;

	/**
	 * Static initializer returns an immutable <code>AvuData</code>
	 * 
	 * @param attribute
	 * @param value
	 * @param unit
	 * @return
	 * @throws JargonException
	 */
	public static AvuData instance(final String attribute, final String value,
			final String unit) throws JargonException {
		return new AvuData(attribute, value, unit);
	}

	private AvuData(final String attribute, final String value,
			final String unit) throws JargonException {
		if (attribute == null || attribute.length() == 0) {
			throw new JargonException("attribute is null or empty");
		}

		if (value == null) {
			throw new JargonException(
					"value is null, leave blank String if empty");
		}

		if (unit == null) {
			throw new JargonException(
					"unit is null, leave blank String if empty");
		}

		this.attribute = attribute;
		this.value = value;
		this.unit = unit;

	}

	public String getAttribute() {
		return attribute;
	}

	public String getValue() {
		return value;
	}

	public String getUnit() {
		return unit;
	}

	@Override
	public String toString() {
		StringBuilder sb = new StringBuilder();
		sb.append("avu data:\n");
		sb.append("   attrib:");
		sb.append(attribute);
		sb.append("\n   value:");
		sb.append(value);
		sb.append("\n   units:");
		sb.append(unit);
		return sb.toString();
	}

}
