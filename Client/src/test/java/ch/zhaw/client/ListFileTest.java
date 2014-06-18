package ch.zhaw.client;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

import java.io.IOException;

import org.junit.Ignore;
import org.junit.Test;

public class ListFileTest extends Testbase {

	@Test
	public void listFile() throws IOException {
		getClient().createFile("test1", "blala");
		getClient().createFile("test2", "blala");
		getClient().createFile("test3", "blala");
		getClient().createFile("test4", "blala");
		getClient().createFile("test5", "blala");
		getClient().createFile("test6", "blala");
		getClient().createFile("test7", "blala");
		getClient().createFile("test8", "blala");
		getClient().createFile("test9", "blala");
		getClient().createFile("test10", "blala");
		getClient().createFile("test11", "blala");
		getClient().createFile("test12", "blala");
		FileListReturnValue rv = getClient().listFiles();
		assertTrue(12 <= rv.getFileCount());
	}
	
	@Test
	@Ignore
	public void listFileWhenEmpty() throws IOException {
		FileListReturnValue rv = getClient().listFiles();
		assertEquals(0,(int) rv.getFileCount());
	}	

}
