/**
 * 
 */
package org.irods.jargon.core.query;

/**
 * Inernally used to classify parts of a query condition when translating from String form to a representation.  This is not immutable, but
 * is not used outside of query translation.
 * 
 * @author Mike Conway - DICE (www.irods.org)
 *
 */
class QueryConditionToken {

	
	public enum TokenType { CONNECTOR, FIELD, LITERAL, OPERATOR}
	private TokenType tokenType;
	private  String value;
	
	public TokenType getTokenType() {
		return tokenType;
	}
	public String getValue() {
		return value;
	}
	public void setTokenType(TokenType tokenType) {
		this.tokenType = tokenType;
	}
	public void setValue(String value) {
		this.value = value;
	}
	
	@Override
	public String toString() {
		StringBuilder sb = new StringBuilder();
		sb.append("QueryConditionToken:");
		sb.append("\n   value:");
		sb.append(value);
		sb.append("\n   type:");
		sb.append(tokenType);
		return sb.toString();
	}
	
	
}
