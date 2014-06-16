package ch.zhaw.client;

import static org.junit.Assert.*;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class ClientTest {
	
	private Client client;

	@Before
	public void setUp() throws Exception {
		client = new Client();
	}

	@After
	public void tearDown() throws Exception {
	}

	@Test
	public void createFileMessage() {
		String message = client.createCreateFileMessage("TestFile", "Blablacontent");
		assertEquals(message, "CREATE TestFile 13\nBlablacontent\n");
	}
	
	@Test
	public void updateFileMessage() {
		String message = client.createUpdateFileMessage("TestFile", "Blablacontent");
		assertEquals(message, "UPDATE TestFile 13\nBlablacontent\n");
	}	
	
	@Test
	public void deleteFileMessage() {
		String message = client.createDeleteFileMessage("TestFile");
		assertEquals(message, "DELETE TestFile\n");
	}	
	
	@Test
	public void readFileMessage() {
		String message = client.createReadFileMessage("TestFile");
		assertEquals(message, "READ TestFile\n");
	}
	
	@Test
	public void listFileMessage() {
		String message = Protocol.COMMAND_LIST;
		assertEquals(message, "LIST\n");
	}		

}
