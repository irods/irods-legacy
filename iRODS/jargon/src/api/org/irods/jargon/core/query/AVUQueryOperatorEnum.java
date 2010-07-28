/**
 * 
 */
package org.irods.jargon.core.query;

/**
 * Describes a metadata query operator.  
 * This will need more work for a full implementation....
 * @author Mike Conway - DICE (www.irods.org)
 */
public enum AVUQueryOperatorEnum {
	BETWEEN("BETWEEN", 8, OperatorClass.VALUE_RANGE), EQUAL("=", 0, OperatorClass.VALUE), GREATER_OR_EQUAL(">=", 5, OperatorClass.VALUE), GREATER_THAN(">", 3, OperatorClass.VALUE),
	IN("IN", 6, OperatorClass.VALUE_IN_ARRAY), LESS_OR_EQUAL("<=", 4, OperatorClass.VALUE), LESS_THAN("<",2, OperatorClass.VALUE),
	LIKE("LIKE", 10, OperatorClass.VALUE), NOT_BETWEEN("NOT_BETWEEN", 9, OperatorClass.VALUE_RANGE), 
	NOT_EQUAL("!=", 1, OperatorClass.VALUE), NOT_IN("NOT_IN", 7, OperatorClass.VALUE_IN_ARRAY),
	NOT_LIKE("NOT_LIKE", 11, OperatorClass.VALUE), 	NUM_GREATER_OR_EQUAL(">=", 18, OperatorClass.VALUE), 
	NUM_LESS_OR_EQUAL("<=", 17, OperatorClass.VALUE), NUM_LESS_THAN("<", 15, OperatorClass.VALUE),
	SOUNDS_LIKE("SOUNDS_LIKE", 11, OperatorClass.VALUE);
	
	private enum OperatorClass {VALUE, VALUE_BY_METADATA_TABLE, VALUE_IN_ARRAY, VALUE_RANGE}
	private OperatorClass operatorClass;
	private int operatorNumericValue;
	private String operatorValue;
	
	AVUQueryOperatorEnum(String operatorValue, int operatorNumericValue, OperatorClass operatorClass) {
		this.operatorValue = operatorValue;
		this.operatorNumericValue = operatorNumericValue;
		this.operatorClass = operatorClass;
	}

	public OperatorClass getOperatorClass() {
		return operatorClass;
	}
	
	public int getOperatorNumericValue() {
		return operatorNumericValue;
	}
	
	public String getOperatorValue() {
		return operatorValue;
	}
}
