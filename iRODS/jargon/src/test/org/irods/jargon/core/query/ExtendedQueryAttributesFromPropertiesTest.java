package org.irods.jargon.core.query;

import static org.junit.Assert.*;
import junit.framework.TestCase;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

public class ExtendedQueryAttributesFromPropertiesTest {

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
	public final void testSetExtendedQueryAttributes() throws Exception {
		ExtendedQueryAttributesFromProperties exAttrs = ExtendedQueryAttributesFromProperties
				.instance();
		if (exAttrs.getExtendedAttributeValueByAttributeValue("10000") == null) {
			TestCase
					.fail("did not successfully load and find property for value 10000...this test case depends on teh default extended_icat_data.properties...did you alter them?");
		}
	}

	@Test
	public final void testGetExtendedAttributeValueByTableAndColumn()
			throws Exception {
		ExtendedQueryAttributesFromProperties exAttrs = ExtendedQueryAttributesFromProperties
				.instance();
		if (exAttrs.getExtendedAttributeValueByTableAndColumn("test", "col1") == null) {
			TestCase
					.fail("did not successfully load and find property for test.col1...this test case depends on teh default extended_icat_data.properties...did you alter them?");
		}
	}

}
