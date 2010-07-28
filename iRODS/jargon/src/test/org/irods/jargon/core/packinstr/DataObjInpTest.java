package org.irods.jargon.core.packinstr;

import static org.junit.Assert.*;

import java.util.ArrayList;
import java.util.List;

import junit.framework.TestCase;

import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;

import edu.sdsc.grid.io.irods.Tag;

public class DataObjInpTest {

	@BeforeClass
	public static void setUpBeforeClass() throws Exception {
	}

	@AfterClass
	public static void tearDownAfterClass() throws Exception {
	}
	
	
	@Test
	public final void testInstanceForOpen() throws Exception {
		DataObjInp dataObjInp = DataObjInp.instanceForOpen("/abspath", DataObjInp.OpenFlags.READ_WRITE);
		TestCase.assertNotNull("data obj inp returned was null", dataObjInp);
	}

	@Test
	public final void testGetParsedTags() throws Exception {
		// use DataObjInp to wrap class
		DataObjInp dataObjInp = DataObjInp.instance("/abspath", DataObjInp.DEFAULT_CREATE_MODE, DataObjInp.OpenFlags.READ, 0L, 0L, 0, "testResource");   
		String packInstr = dataObjInp.getParsedTags();
		TestCase.assertTrue("no packing instruction generated", packInstr.length() > 0);
		
		StringBuilder expectedBuilder = new StringBuilder();
		char ret = '\n';
		expectedBuilder.append("<DataObjInp_PI><objPath>/abspath</objPath>");
		expectedBuilder.append(ret);
		expectedBuilder.append("<createMode>488</createMode>");
		expectedBuilder.append(ret);
		expectedBuilder.append("<openFlags>0</openFlags>");
		expectedBuilder.append(ret);
		expectedBuilder.append("<offset>0</offset>");
		expectedBuilder.append(ret);
		expectedBuilder.append("<dataSize>0</dataSize>");
		expectedBuilder.append(ret);
		expectedBuilder.append("<numThreads>0</numThreads>");
		expectedBuilder.append(ret);
		expectedBuilder.append("<oprType>0</oprType>");
		expectedBuilder.append(ret);
		expectedBuilder.append("<KeyValPair_PI><ssLen>2</ssLen>");
		expectedBuilder.append(ret);
		expectedBuilder.append("<keyWord>dataType</keyWord>");
		expectedBuilder.append(ret);
		expectedBuilder.append("<keyWord>destRescName</keyWord>");
		expectedBuilder.append(ret);
		expectedBuilder.append("<svalue>generic</svalue>");
		expectedBuilder.append(ret);
		expectedBuilder.append("<svalue>testResource</svalue>");
		expectedBuilder.append(ret);
		expectedBuilder.append("</KeyValPair_PI>");
		expectedBuilder.append(ret);
		expectedBuilder.append("</DataObjInp_PI>");
		expectedBuilder.append(ret);
		TestCase.assertEquals("did not get expected packing instruction", expectedBuilder.toString(), packInstr);
		
	}
}
