package ch.zhaw.client;

import static org.junit.Assert.assertEquals;

import java.io.IOException;

import org.junit.Test;

public class ReadFileTest extends Testbase {

	@Test
	public void readFileWithSpaceInContent() throws IOException {
		String fileName = "testFile";
		getClient().createFile(fileName, "Test content");
		String updatedContent = "read content";
		getClient().updateFile(fileName, updatedContent);
		FileReturnValue returnValue = getClient().readFile(fileName);
		assertEquals(getHeaderOfOutput(fileName,updatedContent),returnValue.getRetrunValue());
		assertEquals("read content",returnValue.getContent());
	}

	@Test
	public void readFileWithoutSpaceInContent() throws IOException {
		String fileName = "testFile2";
		getClient().createFile(fileName, "Test_content");
		String updatedContent = "read_content";
		getClient().updateFile(fileName, updatedContent);
		FileReturnValue returnValue = getClient().readFile(fileName);
		assertEquals(getHeaderOfOutput(fileName,updatedContent),returnValue.getRetrunValue());
		assertEquals("read_content", returnValue.getContent());
	}	
	
	@Test
	public void readFileThatNotExists() throws IOException {
		FileReturnValue returnValue = getClient().readFile("noFile");
		assertEquals(Protocol.ANSWER_FAILED_READ,returnValue.getRetrunValue());
	}	
	
	private String getHeaderOfOutput(String fileName, String content) {
		String expectedReturnValue = Protocol.ANSWER_SUCCESS_READ;
		expectedReturnValue = expectedReturnValue.replaceAll(Protocol.PLACEHOLDER_FILENAME, fileName);
		expectedReturnValue = expectedReturnValue.replaceAll(Protocol.PLACEHOLDER_LENGTH, ""+content.length());
		return expectedReturnValue;
	}	

}
