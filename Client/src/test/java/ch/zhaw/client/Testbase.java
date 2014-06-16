package ch.zhaw.client;

import org.junit.After;
import org.junit.Before;

import ch.zhaw.client.utilities.ServerConnector;

public class Testbase {
	
	private Client client;

	@Before
	public void setUp() throws Exception {
		ServerConnector.setServerIP("127.0.0.1");
		ServerConnector.setServerPort(12000);
		client = new Client();
	}

	@After
	public void tearDown() throws Exception {
	}
	
	public Client getClient() {
		return client;
	}

	public void setClient(Client client) {
		this.client = client;
	}	

}
