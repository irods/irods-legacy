package org.irods.jargon.core.query;

import junit.framework.TestCase;

import org.irods.jargon.core.exception.JargonException;
import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

public class ExtendedQueryAttributeTest {

	@BeforeClass
	public static void setUpBeforeClass() throws Exception {
	}

	@AfterClass
	public static void tearDownAfterClass() throws Exception {
	}

	@Before
	public void setUp() throws Exception {
	}

	@After
	public void tearDown() throws Exception {
	}

	@Test
	public final void testInstance() throws Exception {
		ExtendedQueryAttribute attribute = ExtendedQueryAttribute.instance(
				"table", "column", "10000");
		TestCase.assertEquals("table name not correctly set", "table",
				attribute.getTable());
		TestCase.assertEquals("column name not correctly set", "column",
				attribute.getColumn());
		TestCase.assertEquals("attrib name not correctly set", "10000",
				attribute.getAttributeValue());
	}
	
	@Test
	public final void testEquals() throws Exception {
		ExtendedQueryAttribute attribute = ExtendedQueryAttribute.instance(
				"table", "column", "10000");
		ExtendedQueryAttribute otherAttribute = ExtendedQueryAttribute.instance(
				"table", "column", "10000");
		TestCase.assertTrue("should have been equals()", attribute.equals(otherAttribute));
		
	}
	
	@Test
	public final void testNotEquals() throws Exception {
		ExtendedQueryAttribute attribute = ExtendedQueryAttribute.instance(
				"table", "column", "10000");
		ExtendedQueryAttribute otherAttribute = ExtendedQueryAttribute.instance(
				"table", "columnx", "10000");
		TestCase.assertFalse("should not have been equals()", attribute.equals(otherAttribute));
		
	}
	
	
	@Test(expected=JargonException.class)
	public final void testInstanceMissingTable() throws Exception {
		ExtendedQueryAttribute attribute = ExtendedQueryAttribute.instance(
				"", "column", "10000");
		
	}
	
	@Test(expected=JargonException.class)
	public final void testInstanceMissingColumn() throws Exception {
		ExtendedQueryAttribute attribute = ExtendedQueryAttribute.instance(
				"table", "", "10000");
		
	}
	
	@Test(expected=JargonException.class)
	public final void testInstanceMissingAtribute() throws Exception {
		ExtendedQueryAttribute attribute = ExtendedQueryAttribute.instance(
				"table", "column", "");
		
	}
	
	@Test(expected=JargonException.class)
	public final void testInstanceNonNumericAtribute() throws Exception {
		ExtendedQueryAttribute attribute = ExtendedQueryAttribute.instance(
				"table", "column", "xxx");
		
	}

}
