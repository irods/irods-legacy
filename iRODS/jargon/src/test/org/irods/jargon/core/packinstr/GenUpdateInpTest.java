package org.irods.jargon.core.packinstr;

import java.util.ArrayList;
import java.util.List;

import junit.framework.TestCase;

import org.irods.jargon.core.exception.JargonException;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;

public class GenUpdateInpTest {

	@BeforeClass
	public static void setUpBeforeClass() throws Exception {
	}

	@AfterClass
	public static void tearDownAfterClass() throws Exception {
	}

	@Test
	public void testCreateInstanceValid() throws Exception {
		GenUpdateInp.UpdateType updateType = GenUpdateInp.UpdateType.INSERT;
		List<InxVal> vals = new ArrayList<InxVal>();
		InxVal val = InxVal.instance(new Integer(3), "x");
		vals.add(val);
		GenUpdateInp genUpdateInp = GenUpdateInp.instance(updateType, vals);
		TestCase.assertNotNull("null returned from instance method",
				genUpdateInp);
	}

	@Test
	public void testCanGetTagVals() throws Exception {
		GenUpdateInp.UpdateType updateType = GenUpdateInp.UpdateType.INSERT;
		List<InxVal> vals = new ArrayList<InxVal>();
		InxVal val = InxVal.instance(new Integer(3), "x");
		vals.add(val);
		GenUpdateInp genUpdateInp = GenUpdateInp.instance(updateType, vals);
		String tags = genUpdateInp.getParsedTags();
		TestCase.assertNotNull("null returned", tags);
		String expected = "<GeneralUpdateInp_PI><type>23451</type>\n<InxValPair_PI><isLen>1</isLen>\n<inx>3</inx>\n<svalue>x</svalue>\n</InxValPair_PI>\n</GeneralUpdateInp_PI>\n";
		TestCase.assertEquals(expected, tags);
	}

	@Test(expected = JargonException.class)
	public void testCreateInstanceValidNullType() throws Exception {
		List<InxVal> vals = new ArrayList<InxVal>();
		InxVal val = InxVal.instance(new Integer(3), "x");
		vals.add(val);
		GenUpdateInp genUpdateInp = GenUpdateInp.instance(null, vals);
	}

	@Test(expected = JargonException.class)
	public void testCreateInstanceNullVals() throws Exception {
		GenUpdateInp.UpdateType updateType = GenUpdateInp.UpdateType.INSERT;

		GenUpdateInp genUpdateInp = GenUpdateInp.instance(updateType, null);

	}

}
