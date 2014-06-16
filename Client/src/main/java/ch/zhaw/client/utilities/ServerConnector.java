/*
 * $LastChangedRevision: 132 $
 * $LastChangedBy: lickerom $
 * $LastChangedDate: 2012-06-11 19:17:14 +0200 (Mon, 11. Jun 2012) $
 * $HeadURL: https://rom.zhaw.ch/svn/rastro/Trunk/Client/src/rastro/multichannel/utilities/ServerConnector.java $
 */

package ch.zhaw.client.utilities;

import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.ConnectException;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.List;

public class ServerConnector {

	private static String serverIP = "127.0.0.1";
	private static int serverPort = 12000;

	private Socket serverSocket = null;

	public ServerConnector() {
	}

	/**
	 * Erstellt eine {@link Socket} zum Server
	 * 
	 * @return Neuer Socket zum Server
	 * @throws IOException
	 */
	private Socket createSocket() throws IOException, ConnectException,
			UnknownHostException {
		try {
			Socket socket = new Socket(serverIP, serverPort);
			return socket;
		} catch (UnknownHostException e) {
			throw new UnknownHostException(
					"Die IP Adress oder der Port des Servers ist ungültig: "
							+ e.getLocalizedMessage());
		} catch (ConnectException e) {
			throw new ConnectException("Der Server ist nicht erreichbar, "
					+ "bitte überprüfen Sie, ob der Server läuft");
		} catch (IOException e) {
			throw new IOException(
					"Die Verbindung konnte nicht hergestellt werden: "
							+ e.getLocalizedMessage());
		}
	}

	/**
	 * Erstellt eine Verbindung falls noch keine Vorhanden ist
	 * 
	 * @return true = Verbindung besteht, false = Fehler bei der Verbindung
	 * @throws IOException
	 */
	public boolean connect() throws IOException {
		if (serverSocket == null || serverSocket.isClosed()) {
			serverSocket = createSocket();
		}
		if (serverSocket == null) {
			return false;
		}
		return serverSocket.isConnected();
	}

	/**
	 * Trennt die Verbindung zum Server
	 */
	public void disconnect() {
		if (serverSocket != null) {
			try {
				serverSocket.close();
			} catch (IOException e) {
				System.out
						.println("Die Verbindung zum Server konnte nicht geschlossen werden!");
			}
		}
	}

	/**
	 * Sendet die Nachricht falls eine Verbindung zum Server aufgebaut werden
	 * konnte
	 * 
	 * @param message
	 *            Nachricht Objekt
	 * @return 
	 * @throws IOException
	 */
	public List<String> sendMessage(String message) throws IOException {
		BufferedOutputStream outputStream = null;
		BufferedReader inputReader = null;
		List<String> returnValue = new ArrayList<String>();
		if (connect()) {
			try {
				outputStream = new BufferedOutputStream(
						serverSocket.getOutputStream());
				outputStream.write(message.getBytes());
				outputStream.flush();
		
				inputReader = new BufferedReader(new InputStreamReader(
						serverSocket.getInputStream()));
							
				String inputLine;
				while ((inputLine = inputReader.readLine()) != null){
					returnValue.add(inputLine);
				}
			} catch (IOException e) {
				throw new IOException(
						"Probleme mit der Verbindung zum Server: "
								+ e.getLocalizedMessage());
			} finally {
				try {
					if (outputStream != null) {
						outputStream.close();
					}
				} catch (IOException e) {
					e.printStackTrace();
				}
				if (inputReader != null) {
					inputReader.close();
				}
			}

		}
		return returnValue;
	}

	public static String getServerIP() {
		return serverIP;
	}

	public static void setServerIP(String serverIP) {
		ServerConnector.serverIP = serverIP;
	}

	public static int getServerPort() {
		return serverPort;
	}

	public static void setServerPort(int serverPort) {
		ServerConnector.serverPort = serverPort;
	}

}
