/**
 * 
 */
package org.irods.jargon.core.query;

import java.util.ArrayList;
import java.util.List;
import java.util.StringTokenizer;

import org.irods.jargon.core.connection.IRODSServerProperties;
import org.irods.jargon.core.exception.JargonException;
import org.irods.jargon.core.query.SelectField.SelectFieldTypes;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * Translate an IRODSQuery posed as a <code>String</code> query statement (as in
 * iquery) into a format that IRODS understands see
 * lib/core/include/rodsGenQueryNames.h
 * 
 * @author Mike Conway - DICE (www.irods.org)
 */
public class IRODSQueryTranslator {

	private static Logger log = LoggerFactory
			.getLogger(IRODSQueryTranslator.class);
	public static String[] operatorStrings = { "=", "<>", "<", ">", "<=", ">=",
			"in", "not in", "between", "not between", "like", "not like",
			"sounds like", "sounds not like", "TABLE", "num<", "num>", "num<=",
			"num>=", };

	// TODO: add edit of these operator strings?
	// TODO: an error is caused generating conditions if no spaces between
	// operator and field name, way to parse condition out?

	private IRODSServerProperties irodsServerProperties;

	public IRODSQueryTranslator(IRODSServerProperties irodsServerProperties)
			throws JargonException {
		if (irodsServerProperties == null) {
			throw new JargonException("server properties is null");
		}

		this.irodsServerProperties = irodsServerProperties;
	}

	/**
	 * @param conditions
	 * @param tokens
	 * @param i
	 * @param tokenCtr
	 * @param parsedField
	 * @param parsedOperator
	 * @return
	 * @throws JargonQueryException
	 */
	private List<QueryCondition> buildListOfQueryConditionsFromParsedTokens(List<QueryConditionToken> tokens)
			throws JargonQueryException {
		
		QueryCondition queryCondition;
		List<QueryCondition> queryConditions = new ArrayList<QueryCondition>();
		
		int i = 0;
		int tokenCtr = 0;
		
		String parsedField = "";
		String parsedValue = "";
		String parsedOperator = "";
		
		for (QueryConditionToken token : tokens) {

			// I'm interested in the content after the where, and will skip
			// before that
			if (token.getValue().equalsIgnoreCase("where")) {
					throw new JargonQueryException("multiple where statements?, encountered on at the " + tokenCtr + " token after the WHERE");
			}

			// have an and, if I did not finish the last condition, it's an
			// error, otherwise, discard
			if (token.getValue().equalsIgnoreCase("and")) {
				if (i > 0) {
					throw new JargonQueryException(
							"I found an AND operator after an incomplete condition at token " + tokenCtr + " after the where");
				}
				continue;
			}

			// I am now in the conditions, up each of the elements, should be in the form of field, operator, condition(s)
			if (i == 0) {
				// treat as field
				parsedField = token.getValue().trim();
				i++;
			} else if (i == 1) {
				parsedOperator = token.getValue().trim();
				i++;
			} else if (i == 2) {
				parsedValue = token.getValue().trim();
				// TODO: add multiple values for BETWEEN, etc

				if (parsedField.isEmpty() || parsedOperator.isEmpty()
						|| parsedValue.isEmpty()) {
					throw new JargonQueryException(
							"query attribute/value/condition malformed around element:"
									+ tokenCtr);
				}

				queryCondition = QueryCondition.instance(parsedField,
						parsedOperator, parsedValue);
				queryConditions.add(queryCondition);
				i = 0;
			}

			tokenCtr++;
		}

		if (i > 0) {
			throw new JargonQueryException(
					"the last query condition is incomplete");
		}

		return queryConditions;
	}

	protected boolean doesQueryFlagNonDistinct(String query) {
		if (query.toUpperCase().indexOf("NON-DISTINCT") > -1) {
			return true;
		} else {
			return false;
		}
	}

	public TranslatedIRODSQuery getTranslatedQuery(IRODSQuery irodsQuery)
			throws JargonQueryException, JargonException {
		List<String> selects = parseSelectsIntoListOfNames(irodsQuery
				.getQueryString());
		List<QueryCondition> conditions = parseConditionsIntoList(irodsQuery
				.getQueryString());

		// FIXME: condition like x ='14' does not work....need to detect the
		// conditional and compensate by putting spaces around

		List<SelectField> translatedSelects = new ArrayList<SelectField>();
		List<TranslatedQueryCondition> translatedConditions = new ArrayList<TranslatedQueryCondition>();

		boolean isDistinct = true;
		// is this a non-distinct query
		if (doesQueryFlagNonDistinct(irodsQuery.getQueryString())) {
			isDistinct = false;
		}

		// go through selects and get the translation from the 'string' name to
		// the index and other information contained in the enumeration
		int i = 0;
		SelectField translated = null;
		for (String select : selects) {
			translated = translateSelectFieldAsIRODSQueryValue(select
					.toUpperCase());

			// TODO: add extensible translation steps
			if (translated != null) {
				translatedSelects.add(translated);
			} else if (translated == null) {
				throw new JargonQueryException(
						"untranslatable select in position:" + i);
			}

			i++;

		}

		// process conditions
		TranslatedQueryCondition translatedCondition;
		i = 0;
		for (QueryCondition queryCondition : conditions) {
			if (log.isDebugEnabled()) {
				log.debug("translating condition:" + queryCondition);
			}

			// check condition as IRODS data
			translatedCondition = TranslatedQueryCondition.instance(
					RodsGenQueryEnum.getAttributeBasedOnName(queryCondition
							.getFieldName()), queryCondition.getOperator(),
					queryCondition.getValue());

			if (translatedCondition != null) {
				translatedConditions.add(translatedCondition);
				if (log.isDebugEnabled()) {
					log.debug("added query condition:");
					log.debug(translatedCondition.toString());
				}
			} else

			if (translatedCondition == null) {
				throw new JargonQueryException(
						"untranslatable condition in position:" + i
								+ " after the where");
			}

			i++;

		}

		// do a final check to make sure everything translated
		if (translatedSelects.isEmpty()) {
			throw new JargonQueryException("no selects found in query");
		}

		i = 0;
		for (SelectField selectField : translatedSelects) {

			if (selectField == null) {
				throw new JargonQueryException(
						"untranslated select field in position:" + i);
			}

			if (selectField.getSelectFieldNumericTranslation() == null) {
				throw new JargonQueryException(
						"untranslated select field in position:" + i);
			}
			i++;
		}

		i = 0;
		for (TranslatedQueryCondition condition : translatedConditions) {
			if (condition.getColumnNumericTranslation() == null) {
				throw new JargonQueryException(
						"untranslated condition field in position:" + i
								+ " after the WHERE");
			}
			i++;
		}

		return TranslatedIRODSQuery.instance(translatedSelects,
				translatedConditions, irodsQuery, isDistinct);
	}

	protected List<QueryCondition> parseConditionsIntoList(String query)
			throws JargonQueryException {
		List<QueryCondition> conditions = new ArrayList<QueryCondition>();
		
		int idxOfWhere = -1;
		int indexOfWhereUC = query.indexOf(" WHERE ");
		int indexOfWhereLC = query.indexOf(" where ");
		
		if (indexOfWhereUC != -1) {
			idxOfWhere = indexOfWhereUC;
		} else if (indexOfWhereLC != -1) {
				idxOfWhere = indexOfWhereLC;
		}
		
		if (idxOfWhere == -1) {
			log.debug("no where conditions, returning");
			return conditions;
		}
		
		int conditionOffset = idxOfWhere + 7;
		// have a condition, begin parsing into discrete tokens, treating quoted literals as a token.
		List<QueryConditionToken> tokens = tokenizeConditions(query, conditionOffset);
		
		log.debug("query condition tokens were: {}", tokens);
		//evalutate the tokens as components of a condition and return a 'classified' list
		return buildListOfQueryConditionsFromParsedTokens(tokens);

	}

	protected List<String> parseSelectsIntoListOfNames(String query)
			throws JargonQueryException {
		List<String> selectFieldNames = new ArrayList<String>();
		boolean readPastSelect = false;
		boolean haveNotHitWhere = true;
		String token;
		// convert ',' to ' ' for easier tokenizing
		String queryNoCommas = query.replaceAll(",", " ");
		StringTokenizer tokenizer = new StringTokenizer(queryNoCommas, " ");
		int tokenCtr = 0;

		while (tokenizer.hasMoreElements() && haveNotHitWhere) {
			token = tokenizer.nextToken();

			// I'm interested in the content between the select and the where
			if (token.equalsIgnoreCase("select")) {
				readPastSelect = true;
				continue;
			}

			if (!readPastSelect) {
				JargonQueryException qe = new JargonQueryException(
						"error in query, no select detected");
				qe.setQuery(query);
				throw qe;
			}

			if (token.equalsIgnoreCase("where")) {
				haveNotHitWhere = false;
				continue;
			}

			if (token.equalsIgnoreCase("distinct")
					|| token.equalsIgnoreCase("non-distinct")) {
				if (tokenCtr != 1) {
					JargonQueryException qe = new JargonQueryException(
							"distinct/non-distinct must be second token after select");
					qe.setQuery(query);
				}
				continue;
			}

			// have a token that should be a select

			selectFieldNames.add(token.trim());
			tokenCtr++;

		}
		// first statement is select
		return selectFieldNames;

	}

	/**
	 * @param query
	 * @param conditionOffset
	 * @throws JargonQueryException
	 */
	private List<QueryConditionToken> tokenizeConditions(String query, int conditionOffset)
			throws JargonQueryException {
		String conditionString = query.substring(conditionOffset);
		log.debug("conditions in string: {}", conditionString);
		
		// break conditions into tokens and store in an Array
		List<QueryConditionToken> queryTokens = new ArrayList<QueryConditionToken>();
		
		StringBuilder tokenAccum = new StringBuilder();
		
		QueryConditionToken token = null;
		boolean accumulatingQuotedLiteral = false;
		char nextChar = ' ';
		
		// march through what's after the where clause
		for (int i = 0; i < conditionString.length(); i++) {
			
			nextChar = conditionString.charAt(i);
			if (nextChar == ' ') {
				
				// space is delim unless in a quoted literal
				
				if (accumulatingQuotedLiteral) {
					tokenAccum.append(nextChar);
				} else {
					if (tokenAccum.length() > 0) {
						token = new QueryConditionToken();
						token.setValue(tokenAccum.toString());
						queryTokens.add(token);
						tokenAccum = new StringBuilder();
					}
				}
				continue;
			}
				
				// quote either starts or ends accumulation of a quoted literal
				
				if (nextChar == '\'') {
					if (accumulatingQuotedLiteral) {
						// this ends a literal, save the closing quote, and the next space will cause the literal
						// to be tokenized
						tokenAccum.append(nextChar);
						accumulatingQuotedLiteral = false;
					} else {
						if (tokenAccum.length() > 0) {
							// this is an opening quote, but there are characters accumulated before it, error
							throw new JargonQueryException("error in condition at position " + (conditionOffset + i) + " an invalid quote character was encountered");
						} else {
							accumulatingQuotedLiteral = true;
							tokenAccum.append(nextChar);
						}
					}
					continue;
				}
				
				// if not a space or single-quote, just accumulate
				tokenAccum.append(nextChar);
		}
		
		// end of loop that was accumulating tokens, put out the last token
		if (accumulatingQuotedLiteral) {
			throw new JargonQueryException("unclosed quoted literal found in condition");
		}
		
		token = new QueryConditionToken();
		token.setValue(tokenAccum.toString());
		queryTokens.add(token);
		
		return queryTokens;
	}

	/**
	 * Given a textual name, attempt to translate this field as an IRODS
	 * GenQuery field. This method accepts fields that are aggregations, such as
	 * sum(field). Warning: this method returns null if lookup is unsuccessful.
	 * 
	 * @param originalSelectField
	 *            <code>String</code> with query field
	 * @return {@link org.irods.jargon.core.query.SelectField} with the
	 *         translation, or null if not found.
	 * @throws JargonQueryException
	 *             indicates malformed query field.
	 */
	protected SelectField translateSelectFieldAsIRODSQueryValue(
			final String originalSelectField) throws JargonException,
			JargonQueryException {
		String rawField;
		String aggregationComponent;
		SelectFieldTypes selectFieldType;

		if (log.isDebugEnabled()) {
			log.debug("translating select field:" + originalSelectField);
		}

		int parenOpen = originalSelectField.indexOf('(');
		int parenClose = -1;

		// is this a plain select or a computed select like sum()?
		if (parenOpen > -1) {
			parenClose = originalSelectField.indexOf(')');
			if (parenClose == -1) {
				throw new JargonQueryException("malformed select field:"
						+ originalSelectField);
			}

			rawField = originalSelectField.substring(parenOpen + 1, parenClose);

			if (rawField.length() == 0) {
				throw new JargonQueryException(
						"malformed select, aggregation but no field name specified in between ( and )");
			}

			aggregationComponent = originalSelectField.substring(0, parenOpen);

			if (aggregationComponent.equals("SUM")) {
				selectFieldType = SelectFieldTypes.SUM;
			} else if (aggregationComponent.equals("AVG")) {
				selectFieldType = SelectFieldTypes.AVG;
			} else if (aggregationComponent.equals("COUNT")) {
				selectFieldType = SelectFieldTypes.COUNT;
			} else if (aggregationComponent.equals("MIN")) {
				selectFieldType = SelectFieldTypes.MIN;
			} else if (aggregationComponent.equals("MAX")) {
				selectFieldType = SelectFieldTypes.MAX;
			} else {
				throw new JargonQueryException(
						"malformed select, unknown aggregation type of "
								+ aggregationComponent + " in field:"
								+ originalSelectField);
			}

		} else {
			rawField = originalSelectField;
			selectFieldType = SelectFieldTypes.FIELD;
		}

		RodsGenQueryEnum field = RodsGenQueryEnum
				.getAttributeBasedOnName(rawField);
		SelectField.SelectFieldSource source = SelectField.SelectFieldSource.DEFINED_QUERY_FIELD;

		if (field == null) {
			log.debug("retuning null, this is not an IRODS query field");
			return null;
		}

		return SelectField.instance(field, selectFieldType,
				SelectField.SelectFieldSource.DEFINED_QUERY_FIELD);
	}
}
