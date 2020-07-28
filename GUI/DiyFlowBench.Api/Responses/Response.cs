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
using System;

namespace DiyFlowBench.Api.Responses
{
    public class Response
    {
        protected internal Response(Command command, byte[] data)
        {
            if (command == null) throw new ArgumentNullException("command");
            if (data == null) throw new ArgumentNullException("data");
            if (data.Length < 1) throw new ArgumentException("Empty response not supported.");

            if ((data[0] & 0x80) == 0x80) throw new CommandFailedException(command); 
            if (data[0] != command.Identifier) throw new ArgumentException($"Invalid response identifier. (Expected: {(byte)command.Identifier}) (Actual: {data[0]})");
            if (data.Length > 1 && !ValidateChecksum(data)) throw new InvalidChecksumException();

            Command = command;
            Data = data;
        }


        internal static Response BuildResponse(Command command, byte[] data)
        {
            char identifier = ((char)(data[0] & 0x7F));

            switch(identifier)
            {
                case 'V':
                    return new GetVersionResponse(command, data);
                default:
                    throw new UnsupportedCommandIdentiferException(identifier);
            }
        }

        private static bool ValidateChecksum(byte[] data)
        {
            byte checksum = 0;
            for (int x = 0; x < data.Length - 1; x++)
                checksum = (byte)((checksum + data[x]) % byte.MaxValue);
            return checksum == data[data.Length - 1];
        }


        public Command Command { get; private set; }

        protected byte[] Data { get; private set; }

        public bool Success { get { return (Data[0] & 0x80) == 0; } }
    }
}
