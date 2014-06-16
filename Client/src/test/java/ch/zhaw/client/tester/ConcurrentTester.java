package ch.zhaw.client.tester;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import ch.zhaw.client.Client;
import ch.zhaw.client.FileListReturnValue;
import ch.zhaw.client.FileReturnValue;
import ch.zhaw.client.Protocol;
import ch.zhaw.client.ReturnValue;
import ch.zhaw.client.ReturnValueValidator;

public class ConcurrentTester extends Thread {

	public static final String DEFAULT_PREFIX = "concurrentFile";

	private String filePrefix;
	private Client client;
	private boolean status;

	public ConcurrentTester() {
		this(DEFAULT_PREFIX);
	}

	public ConcurrentTester(String filePrefix) {
		this.filePrefix = filePrefix;
		this.client = new Client();
		this.status = true;
	}

	@Override
	public void run() {
		try {
			String file1 = "file1";
			String createContent = "Balbla content";
			if (createFile(file1, createContent)) {
				readFile(file1, createContent);
			}

			String updatedContent = "blaLe _ other content";
			updateFile(file1, updatedContent);
			readFile(file1, updatedContent);

			String file2 = "file2";
			String createContent2 = "Balbla content";
			if (createFile(file2, createContent2)) {
				readFile(file2, createContent2);
			}

			String updatedContent2 = "blaLe _ other content";
			updateFile(file2, updatedContent2);
			readFile(file2, updatedContent2);

			List<String> expectedFiles = new ArrayList<String>();
			expectedFiles.add(filePrefix + file1);
			expectedFiles.add(filePrefix + file2);
			listFiles(expectedFiles);

			deleteFile(file1);
			deleteFile(file2);
		} catch (Exception e) {
			System.out.println("Error in Thread: " + e.getMessage());
			e.printStackTrace();
			setStatus(false);
		}
	}

	public boolean createFile(String fileName, String content)
			throws IOException {
		ReturnValue rv;
		client.getServerConnector().connect();
		rv = client.createFile(filePrefix + fileName, content);
		if (!ReturnValueValidator.validateReturnValue(rv)) {
			setStatus(false);
		}
		if (rv.getRetrunValue().matches(Protocol.ANSWER_SUCCESS_CREATE)) {
			return true;
		}
		return false;
	}

	public void readFile(String fileName, String expectedContent)
			throws IOException {
		FileReturnValue rv;
		rv = client.readFile(filePrefix + fileName);
		if (!ReturnValueValidator.validateReturnValue(rv)) {
			setStatus(false);
		}
		if (!filePrefix.equals(DEFAULT_PREFIX) && !rv.getContent().equals(expectedContent)) {
			System.out.println("READ of " + fileName + ": '" + rv.getContent()
					+ "' does not match to '" + expectedContent + "'");
			setStatus(false);
		}
	}

	public void updateFile(String fileName, String content) throws IOException {
		ReturnValue rv = client.updateFile(filePrefix + fileName, content);
		if (!ReturnValueValidator.validateReturnValue(rv)) {
			setStatus(false);
		}
	}

	private void deleteFile(String fileName) throws IOException {
		ReturnValue rv = client.deleteFile(filePrefix + fileName);
		if (!ReturnValueValidator.validateReturnValue(rv)) {
			setStatus(false);
		}
	}

	public void listFiles(List<String> expectedFiles) throws IOException {
		FileListReturnValue rv = client.listFiles();
		if (!ReturnValueValidator.validateReturnValue(rv)) {
			setStatus(false);
		}
		if (!filePrefix.equals(DEFAULT_PREFIX)
				&& rv.getFileCount() < expectedFiles.size()) {
			System.out.println("LIST: File count: " + rv.getFileCount() + " < "
					+ expectedFiles.size());
			setStatus(false);
		}
		if (!filePrefix.equals(DEFAULT_PREFIX)) {
			for (String fileName : expectedFiles) {
				if (!rv.getFileNames().contains(fileName)) {
					System.out.println("LIST: File " + fileName + " not in "
							+ rv.getFileNames());
					setStatus(false);
				}
			}
		}
	}

	public boolean isSuccessfull() {
		return status;
	}

	public void setStatus(boolean status) {
		int count = 0;
		for (StackTraceElement e : Thread.currentThread().getStackTrace()) {
			count++;
			if (count == 1) {
				continue;
			}
			System.out.println(e.getFileName() + " " + e.getMethodName() + " "
					+ e.getLineNumber());
		}
		if (this.status == true) {
			this.status = status;
		}
	}

}
