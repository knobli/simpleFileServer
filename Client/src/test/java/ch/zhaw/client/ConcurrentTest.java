package ch.zhaw.client;

import static org.junit.Assert.assertTrue;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import org.junit.Before;
import org.junit.Test;

import ch.zhaw.client.tester.ConcurrentTester;
import ch.zhaw.client.utilities.ServerConnector;

public class ConcurrentTest {
	
	@Before
	public void setUp() throws Exception {
//		ServerConnector.setServerIP("160.85.133.78");
//		ServerConnector.setServerPort(7000);
		ServerConnector.setServerIP("127.0.0.1");
		ServerConnector.setServerPort(12000);
	}	

	@Test
	public void concurrentTestWithOnePrefix() throws IOException, InterruptedException {
		ConcurrentTester tester = createSpecificThread("simpleTester");
		tester.join();
        assertTrue(tester.isSuccessfull());
	}
	
	@Test
	public void concurrentTestWithFourDiffPrefixes() throws IOException, InterruptedException {
		testPrefixed(4);
	}
	
	@Test
	public void concurrentTestWith20DiffPrefixes() throws IOException, InterruptedException {
		testPrefixed(20);
	}	
	
	@Test
	public void concurrentTestWith30DiffPrefixes() throws IOException, InterruptedException {
		testPrefixed(30);
	}	
	
	@Test
	public void concurrentTestWith40DiffPrefixes() throws IOException, InterruptedException {
		testPrefixed(40);
	}		
	
	@Test
	public void concurrentTestWith50DiffPrefixes() throws IOException, InterruptedException {
		testPrefixed(50);
	}
	
	private void testPrefixed(int max) throws InterruptedException{
		List<ConcurrentTester> threads = new ArrayList<ConcurrentTester>();
		for (int i = 0; i < max; i++) {
			threads.add(createSpecificThread("tester"+i));
		}
		for(ConcurrentTester tester : threads){
			tester.join();
			assertTrue(tester.isSuccessfull());
		}		
	}

	private ConcurrentTester createSpecificThread(String name) {
		ConcurrentTester tester = new ConcurrentTester(name);
		tester.start();
		return tester;
	}
	
	private ConcurrentTester createThread() {
		ConcurrentTester tester = new ConcurrentTester();
		tester.start();
		return tester;
	}	
	
	@Test
	public void concurrentTest() throws IOException, InterruptedException {
		List<ConcurrentTester> threads = new ArrayList<ConcurrentTester>();
		for (int i = 0; i < 50; i++) {
			threads.add(createThread());
		}
		for(ConcurrentTester tester : threads){
			tester.join();
			assertTrue(tester.isSuccessfull());
		}
	}	
	
	@Test
	public void painConcurrentTest() throws IOException, InterruptedException {
		List<ConcurrentTester> threads = new ArrayList<ConcurrentTester>();
		for (int i = 0; i < 200; i++) {
			threads.add(createThread());
		}
		for(ConcurrentTester tester : threads){
			tester.join();
			assertTrue(tester.isSuccessfull());
		}
	}	

}

