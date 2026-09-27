import pathlib
import subprocess
import sys
import tempfile
import unittest

BIN = pathlib.Path(sys.argv.pop(1)).resolve()
STEP = "P2\n5 5\n255\n0 0 0 255 255\n0 0 0 255 255\n0 0 0 255 255\n0 0 0 255 255\n0 0 0 255 255\n"
EDGES = "P2\n5 5\n255\n0 0 0 0 0\n0 0 255 255 0\n0 0 255 255 0\n0 0 255 255 0\n0 0 0 0 0\n"


class EdgeTests(unittest.TestCase):
    def test_sobel_detects_vertical_step_and_zeros_border(self):
        with tempfile.TemporaryDirectory() as directory:
            source = pathlib.Path(directory) / "step.pgm"
            target = pathlib.Path(directory) / "edges.pgm"
            source.write_text(STEP, encoding="ascii")
            result = subprocess.run([str(BIN), str(source), str(target)], capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(target.read_text(encoding="ascii"), EDGES)

    def test_rejects_truncated_image_without_output(self):
        with tempfile.TemporaryDirectory() as directory:
            source = pathlib.Path(directory) / "bad.pgm"
            target = pathlib.Path(directory) / "edges.pgm"
            source.write_text("P2\n2 2\n255\n0 1 2\n", encoding="ascii")
            result = subprocess.run([str(BIN), str(source), str(target)], capture_output=True, text=True)
            self.assertNotEqual(result.returncode, 0)
            self.assertIn("pixel", result.stderr.lower())
            self.assertFalse(target.exists())

    def test_refuses_to_overwrite_existing_file(self):
        with tempfile.TemporaryDirectory() as directory:
            source = pathlib.Path(directory) / "step.pgm"
            target = pathlib.Path(directory) / "existing.pgm"
            source.write_text(STEP, encoding="ascii")
            target.write_text("keep", encoding="ascii")
            result = subprocess.run([str(BIN), str(source), str(target)], capture_output=True, text=True)
            self.assertNotEqual(result.returncode, 0)
            self.assertEqual(target.read_text(encoding="ascii"), "keep")
            self.assertEqual(list(pathlib.Path(directory).glob("existing.pgm.tmp-*")), [])

    def test_refuses_dangling_symlink_without_partial_output(self):
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            source = root / "step.pgm"
            target = root / "edges.pgm"
            missing = root / "missing.pgm"
            source.write_text(STEP, encoding="ascii")
            try:
                target.symlink_to(missing)
            except (OSError, NotImplementedError):
                self.skipTest("creating symlinks is unavailable on this host")
            result = subprocess.run([str(BIN), str(source), str(target)], capture_output=True, text=True)
            self.assertNotEqual(result.returncode, 0)
            self.assertTrue(target.is_symlink())
            self.assertFalse(missing.exists())
            self.assertEqual(list(root.glob("edges.pgm.tmp-*")), [])


if __name__ == "__main__":
    unittest.main()
