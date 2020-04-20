/****************************************
 * The DIY Flow Bench project
 * A basic flow bench to measure and display volumetric air flow using an Arduino and common automotive MAF sensor.
 * 
 * For more information please visit our GitHub project page: https://github.com/DeeEmm/DIY-Flow-Bench/wiki
 * Or join our Facebook community: https://www.facebook.com/groups/diyflowbench/
 * 
 * This project and all associated files are provided for use under the GNU GPL3 license:
 * https://github.com/DeeEmm/DIY-Flow-Bench/blob/master/LICENSE
 ***/

// Development and release version - Don't forget to update the changelog!!
using DiyFlowBench.Api.Commands;
using DiyFlowBench.Api.Responses;
using DiyFlowBench.Api.Test.Commands;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;

namespace DiyFlowBench.Api.Test.Responses
{
    [TestClass]
    public class ResponseTests
    {
        [TestMethod]
        [ExpectedException(typeof(ArgumentNullException))]
        public void Response_NullCommand_ArguemntNullException()
        {
            //Arrange
            byte[] data = new byte[] { 0x56 };

            //Act
            Response response = new GenericResponse(null, data);
        }

        [TestMethod]
        [ExpectedException(typeof(ArgumentNullException))]
        public void Response_NullData_ArguemntNullException()
        {
            //Arrange
            Command command = new GenericCommand('~');

            //Act
            Response response = new GenericResponse(command, null);
        }

        [TestMethod]
        [ExpectedException(typeof(ArgumentException))]
        public void Response_EmptyData_ArguemntException()
        {
            //Arrange
            Command command = new GenericCommand('~');
            byte[] data = new byte[] { };

            //Act
            Response response = new GenericResponse(command, data);
        }

        [TestMethod]
        [ExpectedException(typeof(ArgumentException))]
        public void Response_InvalidIdentifier_ArguemntException()
        {
            //Arrange
            Command command = new GenericCommand('~');
            byte[] data = new byte[] { 0x20 };

            //Act
            Response response = new GenericResponse(command, data);
        }

        [TestMethod]
        [ExpectedException(typeof(CommandFailedException))]
        public void Response_FailedBitSet_CommandFailedException()
        {
            //Arrange
            Command command = new GenericCommand('~');
            byte[] data = new byte[] { 0xA0 };

            //Act
            Response response = new GenericResponse(command, data);
        }

        [TestMethod]
        public void Response_WhenConstructed_CommandIsSavedCorrectly()
        {
            //Arrange
            Command command = new GenericCommand('~');
            byte[] data = new byte[] { 0x7E };

            //Act
            Response response = new GenericResponse(command, data);

            //Assert
            Assert.AreSame(command, response.Command);
        }
    }
}
